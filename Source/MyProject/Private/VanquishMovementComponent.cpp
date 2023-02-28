// Fill out your copyright notice in the Description page of Project Settings.


#include "VanquishMovementComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "Engine/GameEngine.h"

void UVanquishMovementComponent::PhysFlying(float deltaTime, int32 Iterations) {
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		if (bCheatFlying && Acceleration.IsZero())
		{
			Velocity = FVector::ZeroVector;
		}
		const float Friction = 0.5f * GetPhysicsVolume()->FluidFriction;
		CalcVelocity(deltaTime, Friction, true, GetMaxBrakingDeceleration());
	}

	ApplyRootMotionToVelocity(deltaTime);

	Iterations++;
	bJustTeleported = false;

	FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Adjusted = Velocity * deltaTime;
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (Hit.Time < 1.f)
	{
		const FVector GravDir = FVector(0.f, 0.f, -1.f);
		const FVector VelDir = Velocity.GetSafeNormal();
		const float UpDown = GravDir | VelDir;

		bool bSteppedUp = false;
		if ((FMath::Abs(Hit.ImpactNormal.Z) < 0.2f) && (UpDown < 0.5f) && (UpDown > -0.2f) && CanStepUp(Hit))
		{
			float stepZ = UpdatedComponent->GetComponentLocation().Z;
			bSteppedUp = StepUp(GravDir, Adjusted * (1.f - Hit.Time), Hit);
			if (bSteppedUp)
			{
				OldLocation.Z = UpdatedComponent->GetComponentLocation().Z + (OldLocation.Z - stepZ);
			}
		}

		if (!bSteppedUp)
		{
			//adjust and try again
			HandleImpact(Hit, deltaTime, Adjusted);

			// Thats a Hack, character can only slide left/right
			auto const Adjusted_test = FVector{ Adjusted.X,Adjusted.Y,0.0f };
			SlideAlongSurface(Adjusted_test, (1.f - Hit.Time), Hit.Normal, Hit, true);
		}
	}

}

float UVanquishMovementComponent::SlideAlongSurface(const FVector& Delta, float Time, const FVector& InNormal, FHitResult& Hit, bool bHandleImpact)
{
	if (!Hit.bBlockingHit)
	{
		return 0.f;
	}

	FVector Normal(InNormal);
	
	// We don't want to be pushed up an unwalkable surface.
	if (Normal.Z > 0.f)
	{
		if (!IsWalkable(Hit))
		{
			Normal = Normal.GetSafeNormal2D();
		}
	}
	else if (Normal.Z < -UE_KINDA_SMALL_NUMBER)
	{
		// Don't push down into the floor when the impact is on the upper portion of the capsule.
		if (CurrentFloor.FloorDist < MIN_FLOOR_DIST && CurrentFloor.bBlockingHit)
		{
			const FVector FloorNormal = CurrentFloor.HitResult.Normal;
			const bool bFloorOpposedToMovement = (Delta | FloorNormal) < 0.f && (FloorNormal.Z < 1.f - UE_DELTA);
			if (bFloorOpposedToMovement)
			{
				Normal = FloorNormal;
			}
	
			Normal = Normal.GetSafeNormal2D();
		}
	}

	return Super::SlideAlongSurface(Delta, Time, Normal, Hit, bHandleImpact);
}

