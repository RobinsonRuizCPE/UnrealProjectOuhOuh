// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.h"
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

UCLASS()
class MYPROJECT_API AWeaponBase : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AWeaponBase();

protected:

    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
        float BaseDamage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
        float BulletsPerSeconds;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = 0.0f))
        float BulletSpread;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
        float WeaponRange;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
        FVector TargetPoint;

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

    void Fire();
    void SpawnProjectile(TSubclassOf<AProjectileBase> projectile_class) const;
    void ResetChargedShot();
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

private:
    FVector2D AimDirection;
    FVector3d TargetDirection;

    FTimerHandle TimerHandle_TimeBetweenShots;
    float LastFireTime;
    float TimeBetweenShots;

    bool b_is_charged_shot_ready = true;
    FTimerHandle timer_handle_charged_shot;

    UPROPERTY()
    UCrosshairWidgetBase* CrosshairWidget;

public:

    UFUNCTION(BlueprintCallable, Category = "Weapon")
        void StartFire();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
        void StopFire();

    UFUNCTION(BlueprintCallable, Category = "ChargedShot")
    bool IsChargedShotReady() const { return b_is_charged_shot_ready; }

private:
    void UpdateTargetPoint();
    void UpdateCrosshair();
    AActor* FindRootOwnerActor() const;
};