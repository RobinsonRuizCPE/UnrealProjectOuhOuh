// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.h"
#include "WeaponBase.generated.h"

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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
        float BaseDamage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
        float RateOfFire;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (ClampMin = 0.0f))
        float BulletSpread;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
        float WeaponRange;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
        FVector TargetPoint;

    FTimerHandle TimerHandle_TimeBetweenShots;

    float LastFireTime;

    // Derived from RateOfFire
    float TimeBetweenShots;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance")
        USkeletalMeshComponent* MeshComp;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
        TSubclassOf<UDamageType> DamageType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
        TSubclassOf<AProjectileBase> ProjectileType;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
        FName MuzzleSocketName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
        UNiagaraSystem* MuzzleEffect;

    void Fire();
    void SpawnProjectile() const;
    void PlayFireSound();
    void PlayFireEffects();


    /** single fire sound (bLoopedFireSound not set) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
        USoundBase* FireSound;

    /** finished burst sound (bLoopedFireSound set) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
        USoundCue* FireFinishSound;

    /** fire animations */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
        UAnimMontage* FireAnim;

private:
    FVector2D AimDirection;
    FVector3d TargetDirection;
    UNiagaraComponent* MuzzleEffectComponent;

public:

    UFUNCTION(BlueprintCallable, Category = "Weapon")
        void StartFire();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
        void StopFire();

    UFUNCTION(BlueprintCallable, Category = "Weapon")
        void SetAimDirection(FVector2D const& aim_direction) { AimDirection = aim_direction; };

private:
    void UpdateTargetPoint();
};