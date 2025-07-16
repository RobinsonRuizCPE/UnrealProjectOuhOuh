// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"

#include <Player/SwordAttack/SwordSlashProjectile.h>


#include "EnemyLaser.generated.h"

 /**
  * Laser enemy that uses a beam weapon or high-speed shots.
  */
UENUM(BlueprintType)
enum class ELaserUpdateType : uint8 {
    FollowPlayerTarget,
    RelativeOffsetFromTarget,
    None
};

UCLASS()
class MYPROJECT_API AEnemyLaser : public AEnemyBase
{
    GENERATED_BODY()

public:
    AEnemyLaser();

    virtual void Tick(float DeltaTime) override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

    /** World-space markers for laser path */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Laser Path")
        USceneComponent* LaserStart;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Laser Path")
        USceneComponent* LaserEnd;

    /** Accessors for external logic */
    FVector GetLaserStart() const { return LaserStart->GetComponentLocation(); }
    FVector GetLaserEnd() const { return LaserEnd->GetComponentLocation(); }

    UFUNCTION(BlueprintCallable, Category = AEnemyLaser)
    float GetAdditionalBoxExtentOverride() const { return AdditionalBoxExtentOverride; }

    UFUNCTION(BlueprintCallable, Category = AEnemyLaser)
    bool IsFiringLaser() const { return ActiveSlash.IsValid(); }

    UFUNCTION(BlueprintCallable, Category = AEnemyLaser)
    FVector GetLaserCurrentEndPoint() const {
        if(ActiveSlash.IsValid()) return ActiveSlash->GetLerpPositionCache();
        if (LaserStart) return LaserStart->GetComponentLocation();
        if (PlayerTarget) return PlayerTarget->GetTransform().GetLocation();
        return FVector::Zero();
    }


protected:

    virtual void BeginPlay() override;

    virtual void Attack() override;

    virtual void DecideNextAction() override;

    /** Custom behavior before attack */
    void ChargeLaser();

    /** Spawns laser projectile or triggers laser VFX */
    void FireLaser();

    /** Whether the laser is currently charging */
    bool bIsCharging;

    /** Charge duration before firing */
    UPROPERTY(EditAnywhere, Category = "Laser")
        float ChargeDuration;

    UPROPERTY(EditAnywhere, Category = "Laser")
        float LaserSwipeDuration = 5.f;

    UPROPERTY(EditAnywhere, Category = "Laser")
        FVector LaserUpdateStartOffset;

    UPROPERTY(EditAnywhere, Category = "Laser")
        FVector LaserUpdateEndOffset;

    UPROPERTY(EditAnywhere, Category = "Laser")
        float LaserConstantSpeed = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Laser")
        ELaserUpdateType eLaserUpdateType = ELaserUpdateType::None;

    UPROPERTY(EditAnywhere, Category = "Laser")
        float AdditionalBoxExtentOverride = 0.f;

    UPROPERTY(EditAnywhere, Category = "Laser")
        bool Looping = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
        USoundBase* ChargeSound;

    /** Timer handle for charge/fire sequence */
    FTimerHandle TimerHandle_ChargeDelay;

    /** Niagara effect for charge-up */
    UPROPERTY(EditDefaultsOnly, Category = "Laser|VFX")
        UNiagaraSystem* ChargeEffect;


    /** Reference to active charge effect */
    UPROPERTY()
        UNiagaraComponent* ActiveChargeEffect;

    UPROPERTY(EditDefaultsOnly, Category = "Laser|Audio")
        USoundAttenuation* ChargeSoundAttenuation;

private:
    void UpdatelaserTargets();

private:
    TWeakObjectPtr<ASwordSlashProjectile> ActiveSlash;
};
