// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/EnemyLaser/EnemyLaser.h"

#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "VanquishCharacter.h"

AEnemyLaser::AEnemyLaser()
{
    PrimaryActorTick.bCanEverTick = true;
    bIsCharging = false;
    ChargeDuration = 1.5f; // seconds

    LaserStart = CreateDefaultSubobject<USceneComponent>(TEXT("LaserStart"));
    LaserStart->SetupAttachment(RootComponent);
    LaserStart->SetRelativeLocation(FVector(100, 0, 50));

    LaserEnd = CreateDefaultSubobject<USceneComponent>(TEXT("LaserEnd"));
    LaserEnd->SetupAttachment(RootComponent);
    LaserEnd->SetRelativeLocation(FVector(1000, 0, 50));
}

void AEnemyLaser::BeginPlay()
{
    Super::BeginPlay();
    BeginWaitState(0.5f);

    // Optional: Trigger initial logic or animation
}

void AEnemyLaser::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsDead || bIsCharging)
        return;

    UpdatelaserTargets();
    switch (mCurrentState)
    {
    case EEnemyState::Attack:
        Attack();
        break;

    case EEnemyState::Waiting:
        //UpdateWaitingState(DeltaTime);
        break;
    }
}

void AEnemyLaser::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    if (ActiveSlash.IsValid())
    {
        ActiveSlash->Destroy();
    }
}

void AEnemyLaser::UpdatelaserTargets() {
    switch (eLaserUpdateType) {
    case ELaserUpdateType::FollowPlayerTarget: {
        auto end_location = PlayerTarget->GetTransform().GetLocation() - PlayerTarget->GetTransform().GetRotation() * LaserUpdateEndOffset;
        LaserEnd->SetWorldLocation(end_location);
        break;
    }
    case ELaserUpdateType::RelativeOffsetFromTarget: {
        auto start_location = PlayerTarget->GetTransform().GetLocation() + PlayerTarget->GetTransform().GetRotation() * LaserUpdateStartOffset;
        auto end_location = PlayerTarget->GetTransform().GetLocation() - PlayerTarget->GetTransform().GetRotation() * LaserUpdateEndOffset;
        LaserStart->SetWorldLocation(start_location);
        LaserEnd->SetWorldLocation(end_location);
        break;
    }
    case ELaserUpdateType::None:
        break;
    default:
        break;
    }
}


void AEnemyLaser::Attack()
{
    if (bIsCharging || bIsDead || !PlayerTarget || !ProjectileClass || ActiveSlash.IsValid())
        return;

    float distance = FVector::Distance(PlayerTarget->GetActorLocation(), GetActorLocation());
    if (distance > AttackRange)
    {
        BeginWaitState(0.5f);
        return;
    }

    ChargeLaser();
}

void AEnemyLaser::DecideNextAction() {
    float random_value = FMath::FRand();
    if (random_value < 0.5f) // 50% chance
    {
        BeginWaitState(0.5f);
    }
    else
    {
        mCurrentState = EEnemyState::Attack;
    }
}

void AEnemyLaser::ChargeLaser()
{
    bIsCharging = true;

    if (ChargeEffect)
    {
        ActiveChargeEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
            ChargeEffect,
            GetMesh(),
            "CanonMuzzleSocket",
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget,
            true
        );

        ActiveChargeEffect->SetFloatParameter("Duration", ChargeDuration);
    }

    USoundCue* sound_wave = Cast<USoundCue>(ChargeSound);
    if (sound_wave) {
        float play_rate = sound_wave->Duration > 0 ? (sound_wave->Duration / ChargeDuration) : 1.0f;
        UAudioComponent* audio_comp = UGameplayStatics::SpawnSoundAttached(ChargeSound, GetRootComponent(), NAME_None, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true, 1.0f, 1.f, 0.0f, ChargeSoundAttenuation);
        if (audio_comp) {
            audio_comp->SetPitchMultiplier(play_rate);
        }
    }

    GetWorld()->GetTimerManager().SetTimer(TimerHandle_ChargeDelay, this, &AEnemyLaser::FireLaser, ChargeDuration, false);
}

void AEnemyLaser::FireLaser()
{
    if (ActiveChargeEffect)
    {
        ActiveChargeEffect->Deactivate();
        ActiveChargeEffect = nullptr;
    }

    if (!PlayerTarget || bIsDead || !ProjectileClass)
    {
        bIsCharging = false;
        return;
    }

    FActorSpawnParameters spawn_params;
    spawn_params.Owner = this;
    spawn_params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // Spawn at origin, attachment will handle positioning
    ASwordSlashProjectile* slash = GetWorld()->SpawnActorDeferred<ASwordSlashProjectile>(
        ProjectileClass,
        FTransform::Identity,
        nullptr,
        this,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
        );

    if (!slash) {
        return;
    }

    switch (eLaserUpdateType) {
    case ELaserUpdateType::FollowPlayerTarget: {
        auto start_location = PlayerTarget->GetTransform().GetLocation() + PlayerTarget->GetTransform().GetRotation() * LaserUpdateStartOffset;
        auto end_location = PlayerTarget->GetTransform().GetLocation() - PlayerTarget->GetTransform().GetRotation() * LaserUpdateEndOffset;
        LaserStart->SetWorldLocation(start_location);
        LaserEnd->SetWorldLocation(end_location);
        break;
    }
        case ELaserUpdateType::RelativeOffsetFromTarget: {
            auto start_location = PlayerTarget->GetTransform().GetLocation() + PlayerTarget->GetTransform().GetRotation() * LaserUpdateStartOffset;
            auto end_location = PlayerTarget->GetTransform().GetLocation() - PlayerTarget->GetTransform().GetRotation() * LaserUpdateEndOffset;
            LaserStart->SetWorldLocation(start_location);
            LaserEnd->SetWorldLocation(end_location);
            break;
        }
        case ELaserUpdateType::None:
            break;
        default:
            break;
    }


    slash->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("CanonMuzzleSocket"));
    slash->SetProjectileMaxDistance(100000.f);

    UGameplayStatics::FinishSpawningActor(slash, FTransform::Identity);
    if (FMath::IsNearlyZero(LaserConstantSpeed)) {
        slash->InitializeLaserFromTo(GetMesh(), LaserStart, LaserEnd, "CanonMuzzleSocket", GetMesh()->GetSocketRotation("CanonMuzzleSocket").Vector(), LaserSwipeDuration, AdditionalBoxExtentOverride, Looping);
    }
    else {
        slash->InitializeLaserConstantSpeed(GetMesh(), LaserStart, LaserEnd, "CanonMuzzleSocket", GetMesh()->GetSocketRotation("CanonMuzzleSocket").Vector(), LaserSwipeDuration, LaserConstantSpeed, AdditionalBoxExtentOverride);

    }

    ActiveSlash = slash;
    bIsCharging = false;
    BeginWaitState(LaserSwipeDuration);
}

float AEnemyLaser::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}
