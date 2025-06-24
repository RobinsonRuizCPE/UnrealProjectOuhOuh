// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerFollowSplineComponent.h"
#include "TimerManager.h"

// Sets default values for this component's properties
UPlayerFollowSplineComponent::UPlayerFollowSplineComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPlayerFollowSplineComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPlayerFollowSplineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPlayerFollowSplineComponent::MovePlayerToOffsets(float horizontal_offset, float vertical_offset, float time) {
	GetCurrentOffsets(offset_interp_start_h, offset_interp_start_v);
	offset_interp_target_h = horizontal_offset;
	offset_interp_target_v = vertical_offset;
	offset_interp_elapsed = 0.0f;
	offset_interp_duration = time;
	bEnablePlayerControl = false;

	// Clear any previous interpolation
	GetWorld()->GetTimerManager().ClearTimer(timer_handle_offset_interp);

	// Start a repeating timer to interpolate
	GetWorld()->GetTimerManager().SetTimer(timer_handle_offset_interp, this, &UPlayerFollowSplineComponent::UpdateOffsetInterpolation, 0.01f, true);
}

void UPlayerFollowSplineComponent::UpdateOffsetInterpolation() {
	offset_interp_elapsed += 0.01f;

	float t = FMath::Clamp(offset_interp_elapsed / offset_interp_duration, 0.0f, 1.0f);
	float new_h = FMath::Lerp(offset_interp_start_h, offset_interp_target_h, t);
	float new_v = FMath::Lerp(offset_interp_start_v, offset_interp_target_v, t);

	SetPlayerOffsets(new_h, new_v);

	if (t >= 1.0f)
	{
		bEnablePlayerControl = true;
		GetWorld()->GetTimerManager().ClearTimer(timer_handle_offset_interp);
	}
}

