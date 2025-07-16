// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <Player/SwordAttack/SwordSlashProjectile.h>

#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Actor.h"
#include "Camera/PlayerCameraManager.h"

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "HeatLaserWeapon.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API AHeatLaserWeapon : public AWeaponBase
{
	GENERATED_BODY()

protected:
	AHeatLaserWeapon();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void Fire() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void StartFireImpl() override;
	UFUNCTION()
	void SwordSlashEnded();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
		virtual void StopFireImpl() override;

	UFUNCTION(BlueprintCallable, Category = AEnemyLaser)
		FVector GetLaserCurrentEndPoint() const {
		if (ActiveSlash.IsValid()) return ActiveSlash->GetLerpPositionCache();
		return TargetPoint;
	}

private:
	void ChargeLaser();

	void UpdatePostProcessFade(bool const fade_in, bool const kill_at_end);

protected:
	/** Niagara effect for charge-up */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Laser", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* LaserMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Laser|VFX")
	UNiagaraSystem* ChargeEffect;

	UPROPERTY()
	UNiagaraComponent* ActiveChargeEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* ChargeSound;

	UPROPERTY(EditAnywhere, Category = "Laser")
	float ChargeDuration= 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Laser|Audio")
	USoundAttenuation* ChargeSoundAttenuation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float LaserDuration = 1.f;

	UPROPERTY()
		UPostProcessComponent* PostProcessComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Laser|VFX")
		UMaterialInterface* ChargePostProcessMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Laser|VFX")
		UMaterialInterface* FirePostProcessMaterial;

	UPROPERTY()
		UMaterialInstanceDynamic* ActivePostProcessInstance;

private:
	USceneComponent* LaserStart;
	USceneComponent* LaserEnd;

	FTimerHandle TimerHandle_ChargeDelay;
	FTimerHandle TimerHandle_LaserDuration;

	TWeakObjectPtr<ASwordSlashProjectile> ActiveSlash;

	FTimerHandle fade_timer_handle;
	float post_process_fade_duration = 1.0f;
	float fade_elapsed = 0.0f;
};
