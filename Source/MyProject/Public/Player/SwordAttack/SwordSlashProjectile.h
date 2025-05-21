// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileBase.h"
#include "SwordSlashProjectile.generated.h"

class UCapsuleComponent;

/**
 * 
 */
UCLASS()
class MYPROJECT_API ASwordSlashProjectile : public AProjectileBase
{
	GENERATED_BODY()

public:

	ASwordSlashProjectile();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


	void InitializeSlash(FVector origin, FVector direction, float duration, float speed);

private:
	void MoveSlash(float DeltaTime);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slash")
		FVector slash_direction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slash")
		float life_duration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slash")
		float slash_speed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slash")
		FVector curve_axis = FVector::UpVector; // Axis to rotate around (Z = horizontal arc)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slash")
		float curve_angle_per_sec = 180.0f;     // Degrees per second of rotation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slash")
	UCapsuleComponent* CapsuleComp;


private:
	FRotator slash_start_rot;
	FRotator slash_end_rot;
	FVector initial_location;
	float life_timer = 0.0f;

};
