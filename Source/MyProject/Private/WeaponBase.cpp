#include "WeaponBase.h"
#include <UI/CrosshairWidgetBase.h>

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
#include "Blueprint/WidgetLayoutLibrary.h"
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
    BulletsPerSeconds = 600;
    WeaponRange = 10000.0f;

    PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
    Super::BeginPlay();
    TimeBetweenShots = 1 / BulletsPerSeconds;
    if (CrosshairWidgetClass)
    {
        APlayerController* pc = GetWorld()->GetFirstPlayerController();
        if (pc)
        {
            CrosshairWidget = CreateWidget<UCrosshairWidgetBase>(pc, CrosshairWidgetClass);
            if (CrosshairWidget)
            {
                CrosshairWidget->AddToViewport();
            }
        }
    }
}

void AWeaponBase::Tick(float DeltaTime) {
    UpdateTargetPoint();
    UpdateCrosshair();
}

void AWeaponBase::Fire()
{
    UWorld* pWorld = GetWorld();
    if (!pWorld) {
        return;
    }

    // Will be used to avoid spam
    LastFireTime = pWorld->TimeSeconds;

    SpawnProjectile(ProjectileType);
    PlayFireEffects();
    PlayFireSound(FireSound);
}

void AWeaponBase::SpawnProjectile(TSubclassOf<AProjectileBase> projectile_class) const {
    if (!projectile_class) {
        return;
    }

    UWorld* pWorld = GetWorld();
    if (!pWorld) {
        return;
    }

    auto const projectile_velocity = TargetDirection * 1000.f;
    auto const spawn_location = MeshComp->GetSocketLocation(MuzzleSocketName);
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    auto const spawned_projectile = pWorld->SpawnActor<AProjectileBase>(projectile_class, spawn_location, TargetDirection.ToOrientationRotator(), SpawnParams);
    if (!spawned_projectile) {
        return;
    }

    spawned_projectile->SetOwner(GetParentActor());
    spawned_projectile->SetProjectileCollision(TEXT("CharacterProjectile"));
    spawned_projectile->SetProjectileTrajectory(projectile_velocity);
    spawned_projectile->SetSpawnLocation(GetActorLocation());
    spawned_projectile->SetProjectileMaxDistance(WeaponRange);
}

void AWeaponBase::PlayFireSound(USoundBase* sound) {
    if (!sound) {
        return;
    }

    UWorld* pWorld = GetWorld();
    if (!pWorld) {
        return;
    }

    auto sound_location = MeshComp->GetSocketLocation(MuzzleSocketName);
    UGameplayStatics::PlaySoundAtLocation(pWorld, sound, sound_location);
}

void AWeaponBase::ResetChargedShot() {
    PlayFireSound(ChargeShotReadySound);
    b_is_charged_shot_ready = true;
}

void AWeaponBase::PlayFireEffects()
{
    if (MuzzleEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            MuzzleEffect,
            MeshComp,
            MuzzleSocketName,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget,
            true,     // auto destroy
            true,
            ENCPoolMethod::None,
            true
        );
    }
}

void AWeaponBase::StartFire()
{
    UWorld* pWorld = GetWorld();
    if (!pWorld) {
        return;
    }

    float first_shot_delay = FMath::Max(LastFireTime + TimeBetweenShots - pWorld->TimeSeconds, 0.0f);
    if (b_is_charged_shot_ready && ChargedProjectileType) {
        SpawnProjectile(ChargedProjectileType);
        PlayFireEffects();
        PlayFireSound(FireSoundChargedShot);
        b_is_charged_shot_ready = false;
        first_shot_delay += TimeBetweenShots + 0.3f;
    }
    else {
        GetWorldTimerManager().ClearTimer(timer_handle_charged_shot); // reset cooldown if shot before ready
    }

    GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &AWeaponBase::Fire, TimeBetweenShots, true, first_shot_delay);
}

void AWeaponBase::StopFire()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
    GetWorldTimerManager().SetTimer(timer_handle_charged_shot, this, &AWeaponBase::ResetChargedShot, ChargedShotCooldown, false);
}

void AWeaponBase::UpdateTargetPoint()
{
    FVector crosshair_world_pos, crosshair_world_dir;
    APlayerController* pc = GetWorld()->GetFirstPlayerController();
    if (!pc || !pc->DeprojectMousePositionToWorld(crosshair_world_pos, crosshair_world_dir))
        return;

    FHitResult hit_result;
    FCollisionQueryParams params;
    params.AddIgnoredActor(this);
    if (AActor* meshOwner = GetOwner())
        params.AddIgnoredActor(meshOwner);

    AActor* rootOwner = FindRootOwnerActor();
    if (!rootOwner) return;
    FVector owner_forward = rootOwner->GetActorForwardVector();

    FVector actor_loc = rootOwner->GetActorLocation();
    FVector trace_start = crosshair_world_pos + crosshair_world_dir * 500;
    FVector trace_end = crosshair_world_pos + crosshair_world_dir * 5000;

    bool is_hit = GetWorld()->LineTraceSingleByChannel(hit_result, trace_start, trace_end, ECC_Visibility, params);
    if (CrosshairWidget) {
        CrosshairWidget->SetTargetedActorClass(is_hit ? hit_result.GetActor()->GetClass() : nullptr);
    }

    // If we hit something
    if (is_hit)
    {
        FVector to_hit = (hit_result.ImpactPoint - GetActorLocation()).GetSafeNormal();
        float dot = FVector::DotProduct(owner_forward, to_hit);

        float MinDot = -0.01f;
        if (dot > MinDot)
        {
            TargetPoint = hit_result.ImpactPoint;
            TargetDirection = to_hit;
            return;
        }
        else
        {
            // Clamp to the edge of the allowed cone
            float blend_alpha = 0.8f; // 0 = full owner_forward, 1 = full to_hit
            FVector projected = FVector::VectorPlaneProject(to_hit, owner_forward).GetSafeNormal();
            FVector clamped = FMath::Lerp(projected, to_hit, blend_alpha).GetSafeNormal();
            TargetDirection = clamped;
            TargetPoint = GetActorLocation() + clamped * FVector::Dist(hit_result.ImpactPoint,GetActorLocation());
            return;
        }
    }

    // If no hit, project a point far into the direction of the crosshair
    FVector imagined_point = trace_end;
    FVector to_imagined = (imagined_point - GetActorLocation()).GetSafeNormal();
    TargetPoint = imagined_point;
    TargetDirection = FMath::VInterpTo(owner_forward, to_imagined, 0.95f, 1.0f).GetSafeNormal();
}

void AWeaponBase::UpdateCrosshair()
{
    if (!CrosshairWidget)
        return;

    APlayerController* pc = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!pc)
        return;

    float mouse_x, mouse_y;
    bool got_mouse = pc->GetMousePosition(mouse_x, mouse_y);
    if (!got_mouse)
        return;

    // Get the current viewport scale
    float viewport_scale = UWidgetLayoutLibrary::GetViewportScale(this);

    FVector2D scaled_mouse_pos = FVector2D(mouse_x / viewport_scale, mouse_y / viewport_scale);

    if (UCrosshairWidgetBase* crosshair = Cast<UCrosshairWidgetBase>(CrosshairWidget))
    {
        auto const charged_shot_readyness = b_is_charged_shot_ready ? 1.f : GetWorldTimerManager().GetTimerElapsed(timer_handle_charged_shot) / ChargedShotCooldown;
        crosshair->UpdateChargeCrosshair(charged_shot_readyness);
        crosshair->UpdateCrosshairPosition(scaled_mouse_pos);
    }
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