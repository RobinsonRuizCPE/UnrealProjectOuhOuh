// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileBase.h"
#include "SwordSlashProjectile.generated.h"

class UBoxComponent;

/**
 * 
 */
enum class eSlashMethod {
	RandomArc,
	Laser,
	LaserConstantSpeed,
	None
};

UCLASS()
class MYPROJECT_API ASwordSlashProjectile : public AProjectileBase
{
	GENERATED_BODY()

public:

	ASwordSlashProjectile();

	void InitializeLaserFromTo(USceneComponent * target_origin_component, USceneComponent * target_start_component, USceneComponent* target_end_component, FName socket_name, FVector direction, float duration, float override_additional_box_exent, bool looping = false);

	void InitializeRandomArcSlash(FVector origin, FVector direction, float duration, float speed);
	void InitializeLaserConstantSpeed(USceneComponent* target_origin_component, USceneComponent* target_start_component, USceneComponent* target_end_component, FName socket_name, FVector direction, float duration, float speed, float override_additional_box_extent);
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;



	FVector GetLerpPositionCache() const { return lerped_position_cache; }

private:
	void UpdateRandomArc();

	void UpdateLaser();
	void UpdateConstantSpeed(float delta_time);

	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected:
	/** single fire sound (bLoopedFireSound not set) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* SlashSound;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundAttenuation* SoundAttenuation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slash")
		FVector slash_direction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slash")
		float life_duration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slash")
		FVector curve_axis = FVector::UpVector; // Axis to rotate around (Z = horizontal arc)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slash")
		float curve_angle_per_sec = 180.0f;     // Degrees per second of rotation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slash")
	UBoxComponent* HitBoxComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slash")
		float AddtionalBoxExtent;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* SlashTraceEffect;

	UPROPERTY()
	UNiagaraComponent* SlashTraceEffectComponent;

private:
	FRotator slash_start_rot;
	FRotator slash_end_rot;
	FVector slash_from;
	FVector slash_to;
	FVector initial_location;
	FVector lerped_position_cache;

	FName LaserOriginCompSocketName;

	USceneComponent* LaserOriginComp;
	USceneComponent* LaserStartComp;
	USceneComponent* LaserEndComp;
	float laser_speed = 1.f;

	eSlashMethod eSlashType = eSlashMethod::None;

	float life_timer = 0.0f;

	bool ShouldUpdateBoxExtent = false;
	bool Looping = false;
	bool CurrentLaserLoop = false;

	UPROPERTY()
	UAudioComponent* SlashAudio;
};
