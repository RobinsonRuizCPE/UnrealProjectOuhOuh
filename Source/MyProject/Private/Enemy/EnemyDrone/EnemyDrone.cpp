// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/EnemyDrone/EnemyDrone.h"

#include "VanquishCharacter.h"

#include "Engine/EngineTypes.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Controller.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"

AEnemyDrone::AEnemyDrone()
{
    AntennaHealth.Add("Antenna_UR", 30.0f);
    AntennaHealth.Add("Antenna_UL", 30.0f);
    AntennaHealth.Add("Antenna_LR", 30.0f);
    AntennaHealth.Add("Antenna_LL", 30.0f);
}

void AEnemyDrone::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
    auto const target_player = Cast<AVanquishCharacter>(Target);
    if (!target_player) {
        return;
    }

    switch (CurrentState)
    {
    case EEnemyState::FollowPlayer:
        UpdateFollowPlayer(DeltaTime);
        break;
    case EEnemyState::MoveRandomAdjacent:
        UpdateMoveRandomAdjacent(DeltaTime);
        break;
    case EEnemyState::Attack:
        Attack();
        break;
    }

    UpdateRotation();
}

float AEnemyDrone::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
    {
        const FPointDamageEvent* PointEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);
        FName HitBone = PointEvent->HitInfo.BoneName;
        if (AntennaHealth.Contains(HitBone))
        {
            float& antenna_health = AntennaHealth[HitBone];
            antenna_health -= DamageAmount;

            if (antenna_health <= 0.0f)
            {
                FTransform BoneTransform = GetMesh()->GetBoneTransform(GetMesh()->GetParentBone(HitBone), RTS_World);

                // 1. Outward direction from the bone (flip Y)
                FVector BoneOutward = -BoneTransform.GetUnitAxis(EAxis::Y);

                // 2. Direction of the incoming shot
                FVector ShotDir = PointEvent->ShotDirection.GetSafeNormal();

                // 3. Blend them
                FVector BlendedDir = (BoneOutward * 0.6f + ShotDir * 0.4f).GetSafeNormal();

                // 4. Add upward bias to lift it off visually
                BlendedDir += FVector::UpVector * 0.2f;
                BlendedDir.Normalize();

                // 5. Scale final impulse
                FVector Impulse = BlendedDir * 3000.0f;

                SpawnAntennaDebris(GetMesh()->GetBoneTransform( GetMesh()->GetParentBone(HitBone), RTS_World), Impulse);
                GetMesh()->GetBodyInstance(HitBone)->SetShapeCollisionEnabled(0, ECollisionEnabled::NoCollision);
                GetMesh()->HideBoneByName(HitBone, EPhysBodyOp::PBO_None);
                AntennaHealth.Remove(HitBone);

                return Super::TakeDamage(DamageAmount + AntennaDestructionDamage, DamageEvent, EventInstigator, DamageCauser);
            }
        }
    }

    // Call base class to process health, death, etc.
    return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AEnemyDrone::SpawnAntennaDebris(FTransform const& bone_transform, const FVector& Impulse) {
    if (!AntennaDebrisMesh)
        return;

    UStaticMeshComponent* Debris = NewObject<UStaticMeshComponent>(this);
    Debris->RegisterComponent();
    Debris->SetStaticMesh(AntennaDebrisMesh);
    Debris->SetWorldTransform(bone_transform);
    Debris->SetSimulatePhysics(true);
    Debris->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Debris->SetCollisionObjectType(ECC_PhysicsBody);
    Debris->SetMobility(EComponentMobility::Movable);
    Debris->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

    Debris->SetMassOverrideInKg(NAME_None, 1.0f, true); // Lighten up the part

    Debris->AddImpulse(Impulse, NAME_None, true);

    FVector SpinAxis = FVector::UpVector + FMath::VRand() * 0.3f;
    SpinAxis.Normalize();
    float SpinSpeed = FMath::FRandRange(400.0f, 900.0f);
    Debris->SetPhysicsAngularVelocityInDegrees(SpinAxis * SpinSpeed, true);

    FTimerHandle Timer;
    GetWorld()->GetTimerManager().SetTimer(Timer, [Debris]()
        {
            if (Debris)
            Debris->DestroyComponent();
        }, 3.0f, false);
}
