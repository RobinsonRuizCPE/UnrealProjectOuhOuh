// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/HeatAttack/HeatLaserWeapon.h"
#include "ProjectileBase.h"
#include "VanquishCharacter.h"

#include "NiagaraComponent.h"
#include "NiagaraParameterCollection.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"

AHeatLaserWeapon::AHeatLaserWeapon() {
    LaserMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StickMesh"));
    LaserMesh->SetupAttachment(MeshComp); // or RootComponent if you prefer
    LaserMesh->SetRelativeLocation(FVector(0.f, 0.f, 0.f)); // Adjust as needed
    LaserMesh->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
    LaserMesh->SetVisibility(false);
}

void AHeatLaserWeapon::BeginPlay()
{
    Super::BeginPlay();
}

void AHeatLaserWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (ActiveSlash.IsValid() && LaserEnd) {
        //auto start_location = MeshComp->GetSocketLocation(MuzzleSocketName);
        auto const end_location = TargetPoint + TargetDirection * 3020;
        //LaserStart->SetWorldLocation(start_location);
        LaserEnd->SetWorldLocation(end_location);
        if (auto player = Cast<AVanquishCharacter>(FindRootOwnerActor())) {
            player->SetSwordSlashCurrentPos(ActiveSlash->GetLerpPositionCache());
        }
    }

}

void AHeatLaserWeapon::Fire()
{
    if (ActiveChargeEffect)
    {
        ActiveChargeEffect->Deactivate();
        ActiveChargeEffect = nullptr;
    }

    if (!ProjectileType)
    {
        return;
    }

    if (PostProcessComponent)
    {
        PostProcessComponent->Settings.WeightedBlendables.Array.Empty();
        ActivePostProcessInstance = nullptr;
    }

    // Add fire post process effect
    if (FirePostProcessMaterial)
    {
        ActivePostProcessInstance = UMaterialInstanceDynamic::Create(FirePostProcessMaterial, this);
        PostProcessComponent->AddOrUpdateBlendable(ActivePostProcessInstance);
        PostProcessComponent->BlendWeight = 1.0f;

        fade_elapsed = 0.0f;
        post_process_fade_duration = LaserDuration;
        GetWorld()->GetTimerManager().SetTimer(
            fade_timer_handle,
            FTimerDelegate::CreateLambda([this]()
                {
                    UpdatePostProcessFade(false, true);
                }),
            0.016f, // ~60 FPS
                    true
                    );
    }

    FActorSpawnParameters spawn_params;
    spawn_params.Owner = this;
    spawn_params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    TSubclassOf<ASwordSlashProjectile> slash_type = Cast<UClass>(ProjectileType);
    if (!slash_type) {
        return;
    }

    // Spawn at origin, attachment will handle positioning
    ASwordSlashProjectile* slash = GetWorld()->SpawnActorDeferred<ASwordSlashProjectile>(
        slash_type.Get(),
        FTransform::Identity,
        this
    );

    if (!slash) {
        return;
    }

    auto start_location = TargetPoint + TargetDirection * 3019;
    auto end_location = TargetPoint + TargetDirection*3020;
    LaserStart->SetWorldLocation(start_location);
    LaserEnd->SetWorldLocation(end_location); 

    slash->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName(MuzzleSocketName));
    slash->SetProjectileMaxDistance(100000.f);

    UGameplayStatics::FinishSpawningActor(slash, FTransform::Identity);
    slash->InitializeLaserConstantSpeed(MeshComp, LaserStart, LaserEnd, MuzzleSocketName, MeshComp->GetSocketRotation(MuzzleSocketName).Vector(), LaserDuration, 2000, 0.f);

    ActiveSlash = slash;
    LaserMesh->SetVisibility(true);
    ActiveSlash->OnSlashDestroyed.AddDynamic(this, &AHeatLaserWeapon::SwordSlashEnded);
}

void AHeatLaserWeapon::StartFireImpl()
{
    if (IsWeaponFiring()) {
        return;
    }

    UWorld* pWorld = GetWorld();
    if (!pWorld) {
        return;
    }

    if (GetHeatSystem() && GetHeatSystem()->GetGlobalHeatLevel() < 1) {
        return;
    }

    if (!VanquishRootOwner) {
        return;
    }

    if (!VanquishRootOwner->HasAbilityFlag(ECharacterAbilityFlags::CanHeatAttack)) {
        return;
    }

    VanquishRootOwner->RemoveAbilityFlags({ ECharacterAbilityFlags::CanDodge, ECharacterAbilityFlags::CanMeleeAttack, ECharacterAbilityFlags::CanShoot,ECharacterAbilityFlags::CanHeatAttack });

    IsFiring = true;
    LaserStart = NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("LaserStart"));
    LaserStart->RegisterComponent();

    LaserEnd = NewObject<USceneComponent>(this, USceneComponent::StaticClass(), TEXT("LaserEnd"));
    LaserEnd->RegisterComponent();

    ChargeLaser();
}

void AHeatLaserWeapon::SwordSlashEnded() {
    Super::StopFire();
    GetHeatSystem()->DecreaseGlobalHeatLevel();

    VanquishRootOwner->AddAbilityFlags({ ECharacterAbilityFlags::CanDodge, ECharacterAbilityFlags::CanMeleeAttack, ECharacterAbilityFlags::CanShoot,ECharacterAbilityFlags::CanHeatAttack });
}

void AHeatLaserWeapon::StopFireImpl() {
    IsFiring = false;
    LaserMesh->SetVisibility(false);
    LaserStart->DestroyComponent(); // cleanly removes it from the world and unregisters it
    LaserStart = nullptr;

    LaserEnd->DestroyComponent(); // cleanly removes it from the world and unregisters it
    LaserEnd = nullptr;
}

void AHeatLaserWeapon::ChargeLaser()
{
    if (ChargeEffect)
    {
        ActiveChargeEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
            ChargeEffect,
            RootComponent,
            MuzzleSocketName,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget,
            true
        );

        ActiveChargeEffect->SetFloatParameter("Duration", ChargeDuration);
    }


    if (ChargePostProcessMaterial)
    {
        ActivePostProcessInstance = UMaterialInstanceDynamic::Create(ChargePostProcessMaterial, this);

        PostProcessComponent = NewObject<UPostProcessComponent>(this);
        PostProcessComponent->bUnbound = true; // Affect the whole screen
        PostProcessComponent->RegisterComponent();
        PostProcessComponent->AddOrUpdateBlendable(ActivePostProcessInstance);
        PostProcessComponent->BlendWeight = 1.0f;
        fade_elapsed = 0.0f;
        post_process_fade_duration = ChargeDuration;
        GetWorld()->GetTimerManager().SetTimer(
            fade_timer_handle,
            FTimerDelegate::CreateLambda([this]()
                {
                    UpdatePostProcessFade(true, false);
                }),
            0.016f, // ~60 FPS
            true
         );
    }

    USoundCue* sound_wave = Cast<USoundCue>(ChargeSound);
    if (sound_wave) {
        float play_rate = sound_wave->Duration > 0 ? (sound_wave->Duration / ChargeDuration) : 1.0f;
        UAudioComponent* audio_comp = UGameplayStatics::SpawnSoundAttached(ChargeSound, MeshComp, NAME_None, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true, 1.0f, 1.f, 0.0f, ChargeSoundAttenuation);
        if (audio_comp) {
            audio_comp->SetPitchMultiplier(play_rate);
        }
    }

    GetWorld()->GetTimerManager().SetTimer(TimerHandle_ChargeDelay, this, &AHeatLaserWeapon::Fire, ChargeDuration, false);
}

void AHeatLaserWeapon::UpdatePostProcessFade(bool const fade_in, bool const kill_at_end)
{
    if (!ActivePostProcessInstance)
        return;

    fade_elapsed += 0.016f;
    float alpha = FMath::Clamp(fade_elapsed / post_process_fade_duration, 0.0f, 1.0f);
    float current_blend = fade_in ? FMath::Lerp(0.0f, 1.0f, alpha) : FMath::Lerp(1.0f, 0.0f, alpha);

    ActivePostProcessInstance->SetScalarParameterValue(FName("Intensity"), current_blend);

    if (alpha >= 1.0f)
    {
        GetWorld()->GetTimerManager().ClearTimer(fade_timer_handle);
        ActivePostProcessInstance = nullptr;

        if (kill_at_end) {
            if (PostProcessComponent)
            {
                PostProcessComponent->Settings.WeightedBlendables.Array.Empty();
                PostProcessComponent->DestroyComponent();
                PostProcessComponent = nullptr;
            }
        }
    }
}