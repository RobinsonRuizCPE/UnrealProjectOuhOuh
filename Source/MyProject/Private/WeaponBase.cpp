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
    // Will be used to avoid spam
    LastFireTime = GetWorld()->TimeSeconds;

    UpdateTargetPoint();
    SpawnProjectile();
    PlayFireEffects();
    PlayFireSound();
}

void AWeaponBase::SpawnProjectile() const {
    auto const projectile_velocity = TargetDirection * 1000.f;
    auto const spawned_projectile = GetWorld()->SpawnActor<AProjectileBase>(ProjectileType, GetActorLocation(), TargetDirection.ToOrientationRotator());
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

    auto sound_location = MeshComp->GetSocketLocation(MuzzleSocketName);
    UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, sound_location);
}


void AWeaponBase::PlayFireEffects()
{
    if (!MuzzleEffect) {
        return;
    }
}

void AWeaponBase::StartFire()
{
    float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
    GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &AWeaponBase::Fire, TimeBetweenShots, true, FirstDelay);
    MuzzleEffectComponent->ToggleVisibility(true);
}

void AWeaponBase::StopFire()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
    MuzzleEffectComponent->ToggleVisibility(true);
}

void AWeaponBase::UpdateTargetPoint()
{
    // Get the world space position of the crosshair
    FVector crosshair_world_pos, crosshair_world_dir;
    GetWorld()->GetFirstPlayerController()->DeprojectMousePositionToWorld(crosshair_world_pos, crosshair_world_dir);
    
    // Trace from the weapon to the point of intersection
    FHitResult hit_result;
    FVector trace_start = crosshair_world_pos;
    FVector trace_end = crosshair_world_pos + crosshair_world_dir * 10000.0f; // extend the trace far enough to hit anything in the scene
    FCollisionQueryParams params;
    params.AddIgnoredActor(this); // ignore the weapon itself
    auto const is_hit = GetWorld()->LineTraceSingleByChannel(hit_result, trace_start, trace_end, ECC_Visibility, params);

    // Calculate the direction of the projectile based on the result of the trace
    if (is_hit) {
        // If we hit something, aim at the point of intersection
        TargetPoint = (hit_result.ImpactPoint);
    }
    else {
        TargetPoint = (trace_end - crosshair_world_dir * 7000.0f);
    }

    TargetDirection = (TargetPoint - GetActorLocation()).GetSafeNormal();
}
