#include "WeaponBase.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraParameterCollection.h"
#include "Components/ArrowComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;

    ProjectileType = AProjectileBase::StaticClass();

    MuzzleSocketName = "Muzzle";

    BaseDamage = 20.0f;
    BulletSpread = 2.0f;
    RateOfFire = 600;
    WeaponRange = 10000.0f;
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
    Super::BeginPlay();
    TimeBetweenShots = 60 / RateOfFire;

    TArray<FNiagaraVariable> UserParameters;
    if (MuzzleEffect) {
        MuzzleEffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleEffect, MeshComp, MuzzleSocketName, FVector{}, FRotator{}, EAttachLocation::Type::SnapToTarget, true, true, ENCPoolMethod::None, true);
        MuzzleEffectComponent->SetFloatParameter(FName("Delay"), 0.1f);
        MuzzleEffectComponent->SetVisibility(false, true);
    }
}

void AWeaponBase::Fire()
{
    UWorld* pWorld = GetWorld();
    if (!pWorld) {
        return;
    }

    // Will be used to avoid spam
    LastFireTime = pWorld->TimeSeconds;

    UpdateTargetPoint();
    SpawnProjectile();
    PlayFireEffects();
    PlayFireSound();
}

void AWeaponBase::SpawnProjectile() const {
    if (!ProjectileType) {
        return;
    }

    UWorld* pWorld = GetWorld();
    if (!pWorld) {
        return;
    }

    auto const projectile_velocity = TargetDirection * 1000.f;
    auto const spawn_location = MeshComp->GetSocketLocation(MuzzleSocketName);
    auto const spawned_projectile = pWorld->SpawnActor<AProjectileBase>(ProjectileType, spawn_location, TargetDirection.ToOrientationRotator());
    spawned_projectile->SetOwner(GetParentActor());
    spawned_projectile->SetProjectileCollision(TEXT("CharacterProjectile"));
    spawned_projectile->SetProjectileTrajectory(projectile_velocity);
    spawned_projectile->SetSpawnLocation(GetActorLocation());
    spawned_projectile->SetProjectileMaxDistance(WeaponRange);
}

void AWeaponBase::PlayFireSound() {
    if (!FireSound) {
        return;
    }

    UWorld* pWorld = GetWorld();
    if (!pWorld) {
        return;
    }

    auto sound_location = MeshComp->GetSocketLocation(MuzzleSocketName);
    UGameplayStatics::PlaySoundAtLocation(pWorld, FireSound, sound_location);
}


void AWeaponBase::PlayFireEffects()
{
    if (MuzzleEffectComponent) {
        MuzzleEffectComponent->Activate(true);
    }
}

void AWeaponBase::StartFire()
{
    UWorld* pWorld = GetWorld();
    if (!pWorld) {
        return;
    }

    float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - pWorld->TimeSeconds, 0.0f);
    GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &AWeaponBase::Fire, TimeBetweenShots, true, FirstDelay);
    MuzzleEffectComponent->ToggleVisibility(true);
}

void AWeaponBase::StopFire()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
    if (MuzzleEffectComponent) {
        MuzzleEffectComponent->ToggleVisibility(false);
    }
}

void AWeaponBase::UpdateTargetPoint()
{
    FVector crosshair_world_pos, crosshair_world_dir;
    APlayerController* pc = GetWorld()->GetFirstPlayerController();
    if (!pc || !pc->DeprojectMousePositionToWorld(crosshair_world_pos, crosshair_world_dir))
        return;

    FVector trace_start = crosshair_world_pos;
    FVector trace_end = trace_start + crosshair_world_dir * WeaponRange;

    FHitResult hit_result;
    FCollisionQueryParams params;
    params.AddIgnoredActor(this);
    if (AActor* meshOwner = GetOwner())
        params.AddIgnoredActor(meshOwner);

    AActor* rootOwner = FindRootOwnerActor();
    if (!rootOwner) return;
    FVector owner_forward = rootOwner->GetActorForwardVector();

    bool is_hit = GetWorld()->LineTraceSingleByChannel(hit_result, trace_start, trace_end, ECC_Visibility, params);

    // If we hit something
    if (is_hit)
    {
        FVector to_hit = (hit_result.ImpactPoint - GetActorLocation()).GetSafeNormal();
        float dot = FVector::DotProduct(owner_forward, to_hit);

        float MinDot = -0.5f;
        if (dot > MinDot)
        {
            TargetPoint = hit_result.ImpactPoint;
            TargetDirection = to_hit;
            return;
        }
        else
        {
            // Clamp to the edge of the allowed cone
            FVector projected = FVector::VectorPlaneProject(to_hit, owner_forward).GetSafeNormal();
            FVector clamped = (owner_forward * MinDot + projected * FMath::Sqrt(1 - MinDot * MinDot)).GetSafeNormal();

            TargetDirection = clamped;
            TargetPoint = GetActorLocation() + clamped * WeaponRange;
            return;
        }
    }

    // If no hit, project a point far into the direction of the crosshair
    FVector imagined_point = trace_end;
    FVector to_imagined = (imagined_point - GetActorLocation()).GetSafeNormal();
    float dot_imagined = FVector::DotProduct(owner_forward, to_imagined);

    if (dot_imagined > 0.2f)
    {
        TargetPoint = imagined_point;
        TargetDirection = FMath::VInterpTo(owner_forward, to_imagined, 0.5f, 1.0f).GetSafeNormal();;
        return;
    }

    // Final fallback: straight ahead
    FVector muzzle_forward = MeshComp->GetSocketRotation(MuzzleSocketName).Vector();
    TargetDirection = muzzle_forward;
    TargetPoint = GetActorLocation() + TargetDirection * WeaponRange;
}

AActor* AWeaponBase::FindRootOwnerActor() const
{
    const USceneComponent* parentComponent = Cast<USceneComponent>(GetRootComponent()->GetAttachParent());

    while (parentComponent)
    {
        AActor* outerActor = parentComponent->GetOwner();
        if (outerActor && outerActor != this)
        {
            return outerActor;
        }

        parentComponent = parentComponent->GetAttachParent();
    }

    return nullptr;
}