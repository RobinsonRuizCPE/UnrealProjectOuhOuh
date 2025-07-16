// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.h"
#include <Player/HeatSystemComponent.h>

#include "WeaponBase.generated.h"

class UCrosshairWidgetBase;
class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class AWeaponBase;
class USoundCue;
class UAnimMontage;
class UAudioComponent;
class AActor;
class UNiagaraSystem;
class UNiagaraComponent;
class AVanquishCharacter;

UCLASS()
class MYPROJECT_API AWeaponBase : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AWeaponBase();

    // Public so it can be reset for different reasons (dodge, ...)
    void ResetChargedShot();

    void SetHeatSystemComponent(UHeatSystemComponent* heat_system);

protected:

    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;

    UHeatSystemComponent* GetHeatSystem() { return HeatSystem; }

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    bool IsWeaponFiring() const { return IsFiring; }

    AActor* FindRootOwnerActor() const;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
        float BaseDamage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
        float BulletsPerSeconds;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = 0.0f))
        float BulletSpread;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
        float WeaponRange;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
        float ProjectileVelocityFactorAtStart = 1.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponHeat")
        float HeatAmount = 0.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponHeat")
        float HeatDuration = 0.f;

    bool IsFiring = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponHeat")
        EHeatDecayType HeatDecayType = EHeatDecayType::QuadraticEaseIn;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
        FVector TargetPoint;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
        FVector TargetDirection;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance")
        USkeletalMeshComponent* MeshComp;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
        TSubclassOf<UDamageType> DamageType;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
        TSubclassOf<UCrosshairWidgetBase> CrosshairWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
        TSubclassOf<AProjectileBase> ProjectileType;

    // WeaponBase.h

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChargedShot")
        TSubclassOf<AProjectileBase> ChargedProjectileType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ChargedShot")
        float ChargedShotCooldown = 3.0f;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
        FName MuzzleSocketName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
        UNiagaraSystem* MuzzleEffect;

    virtual void Fire();
    virtual void SpawnProjectile(TSubclassOf<AProjectileBase> projectile_class) const;
    void PlayFireSound(USoundBase* sound);
    void PlayFireEffects();


    /** single fire sound (bLoopedFireSound not set) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
        USoundBase* FireSound;

    /** single fire sound (bLoopedFireSound not set) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
        USoundBase* FireSoundChargedShot;

    /** finished burst sound (bLoopedFireSound set) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
        USoundBase* ChargeShotReadySound;

    /** fire animations */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
        UAnimMontage* FireAnim;

    AVanquishCharacter* VanquishRootOwner;

private:
    FVector2D AimDirection;
    AActor* SelectedTarget;

    FTimerHandle TimerHandle_TimeBetweenShots;
    float LastFireTime;
    float TimeBetweenShots;
    float ProjectileVelocity = 0.f;
    float ProjectileVelocitySpeedAlongSpline = 0.f;

    bool b_is_charged_shot_ready = true;
    FTimerHandle timer_handle_charged_shot;

    UHeatSystemComponent* HeatSystem;

    UPROPERTY()
    UCrosshairWidgetBase* CrosshairWidget;

public:

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void StartFire();
    virtual void StartFireImpl();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void StartFireAtTarget(AActor* target);
    virtual void StartFireAtTargetImpl(AActor* target);

    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void StopFire();
    virtual void StopFireImpl();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
        void SetAdditionalProjectileVelocitySpeedAlongSpline(float additional_velocity) {
        ProjectileVelocitySpeedAlongSpline = additional_velocity;
    }

    UFUNCTION(BlueprintCallable, Category = "ChargedShot")
    bool IsChargedShotReady() const { return b_is_charged_shot_ready; }

private:
    void UpdateTargetPoint();
    void UpdateCrosshair();

    UFUNCTION()
    void HandleHeatLevelChanged(int32 new_heat_level, bool is_increase);
};