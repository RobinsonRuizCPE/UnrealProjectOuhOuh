// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileBase.h"

#include "Components/MeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "EnemyBase.h"
#include "VanquishCharacter.h"

// Sets default values
AProjectileBase::AProjectileBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetVisibility(true);
    CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnOverlap);
    CollisionSphere->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
    SetRootComponent(CollisionSphere);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->InitialSpeed = 3000.0f;
    ProjectileMovementComponent->MaxSpeed = 3000.0f;
    ProjectileMovementComponent->bInitialVelocityInLocalSpace = false;
    ProjectileMovementComponent->bRotationFollowsVelocity = false;
    ProjectileMovementComponent->bShouldBounce = false;
    ProjectileMovementComponent->Bounciness = 0.0f;
    ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
}

void AProjectileBase::SetProjectileTrajectory(FVector const& world_direction)
{
    ProjectileMovementComponent->Velocity = world_direction * VelocityFactor;
    if (TraceEffectComponent)
    {
        FVector world_dir = ProjectileMovementComponent->Velocity.GetSafeNormal();
        FVector local_dir = TraceEffectComponent->GetComponentTransform().InverseTransformVectorNoScale(world_dir);

        FVector beam_start = local_dir * 100.f;
        FVector beam_end = -local_dir * 700.f;

        TraceEffectComponent->SetVectorParameter("BeamStart", beam_start);
        TraceEffectComponent->SetVectorParameter("BeamEnd", beam_end);
    }
}

// Called when the game starts or when spawned
void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
    SpawnLocation = GetActorLocation();
    if(GetOwner())
    {
        CollisionSphere->IgnoreActorWhenMoving(GetOwner(), true);
    }

    if (TraceEffect) {
        TraceEffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            TraceEffect,
            CollisionSphere,           // Attach to the mesh so it follows properly
            NAME_None,                // Attach to socket name, none in this case
            FVector::ZeroVector,      // Relative location
            FRotator::ZeroRotator,    // Relative rotation
            EAttachLocation::SnapToTarget,
            true                      // Auto destroy with parent
        );

        TraceEffectComponent->SetAutoActivate(false); // Don't play until we tell it
    }

    if (TraceEffect && TraceEffectComponent) {
        TraceEffectComponent->SetAsset(TraceEffect);
        TraceEffectComponent->Activate(true);
    }
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
    // Do not hit the same boneless-actor ( :p ) twice
    if (Hit.BoneName.IsNone()) {
        if (HitActors.Contains(OtherActor))
            return;

        HitActors.Add(OtherActor);
        HandleProjectileImpact(OtherActor, Hit.ImpactPoint, Hit);
        return;
    }

    // Do not hit the same bone twice
    auto const hit_key = TPair<TWeakObjectPtr<AActor>, FName>(OtherActor, Hit.BoneName);
    if (HitBones.Contains(hit_key))
        return;

    HitBones.Add(hit_key);
    HandleProjectileImpact(OtherActor, Hit.ImpactPoint, Hit);
}

void AProjectileBase::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
    // Ghost hit result for god knows why ???
    if (SweepResult.Location.IsNearlyZero(0.01)) {
        return;
    }

    // Do not hit the same boneless-actor ( :p ) twice
    if (SweepResult.BoneName.IsNone()) {
        if (HitActors.Contains(OtherActor))
            return;

        HitActors.Add(OtherActor);
        HandleProjectileImpact(OtherActor, SweepResult.ImpactPoint, SweepResult);
        return;
    }

    // Do not hit the same bone twice
    auto const hit_key = TPair<TWeakObjectPtr<AActor>, FName>(OtherActor, SweepResult.BoneName);
    if (HitBones.Contains(hit_key))
        return;

    HitBones.Add(hit_key);
    HandleProjectileImpact(OtherActor, SweepResult.ImpactPoint, SweepResult);
}

// Called every frame
void AProjectileBase::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);

    if (ComputeTraveledDistance() >= MaxRange) {
        Destroy();
    }
}

void AProjectileBase::HandleProjectileImpact(AActor* OtherActor, FVector const& ImpactPoint, const FHitResult& HitResult)
{

    if (HitEffect) {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, ImpactPoint);
    }

    if (HitSound) {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, ImpactPoint);
    }

    if (TraceEffectComponent) {
        TraceEffectComponent->Deactivate();
    }

    auto const damaged = UGameplayStatics::ApplyPointDamage(OtherActor, ProjectileDamage, ProjectileMovementComponent->Velocity, HitResult, GetInstigatorController(), this, UDamageType::StaticClass());
    if (FMath::IsNearlyZero(damaged)) {
        // -- DID NOT TAKE DAMAGE (dodge, ... ) CLEANUP HIT CONTAINERS --
        if (HitResult.BoneName.IsNone()) {
            HitActors.Remove(OtherActor);
        }
        else {
            auto const hit_key = TPair<TWeakObjectPtr<AActor>, FName>(OtherActor, HitResult.BoneName);
            HitBones.Remove(hit_key);
        }
    }

    if (!PiercingShot) {
        Destroy();
    }
}

float const AProjectileBase::ComputeTraveledDistance() {
    return FVector::Distance(SpawnLocation, GetActorLocation());
}

void AProjectileBase::SetProjectileCollision(FName const InCollisionProfileName) {
    CollisionSphere->SetCollisionProfileName(InCollisionProfileName);
}
