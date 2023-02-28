// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "VanquishMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API UVanquishMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	

public :

	UVanquishMovementComponent() : UCharacterMovementComponent() {};

protected:
	void PhysFlying(float deltaTime, int32 Iterations) override;

	/** Custom version of SlideAlongSurface that handles different movement modes separately; namely during walking physics we might not want to slide up slopes. */
	float SlideAlongSurface(const FVector& Delta, float Time, const FVector& Normal, FHitResult& Hit, bool bHandleImpact) override;
};
