// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerFollowSplineComponent.generated.h"


UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYPROJECT_API UPlayerFollowSplineComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerFollowSplineComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Blueprint-callable methods (implemented in Blueprint)
    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spline|Offset")
    void AddVerticalOffset(float NewParam);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spline|Offset")
        void AddHorizontalOffset(float NewParam);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spline|Movement")
        void UpdateDistanceAlongSpline(float DeltaTime);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spline|Movement")
        void UpdateCharacterTransformOnSpline();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spline|Camera")
        void UpdateCameraOffset();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spline|Movement")
        void UpdateStandardFlyingMovement();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spline|Offset")
        void DistributeOffset();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spline|WallSlide")
        void SetWallSlideSide(FVector const& ImpactNormal);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spline|Transform")
        void SetActorTransformAlongSpline(FHitResult& hit_result);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spline|Misc")
        void NewFunction(
            const FVector& ImpactNormal,
            const FVector& RightVecWanted,
            const FVector& UpVecWanted,
            const FVector& ImpactPoint,
            float& NewHorizontalOffset,
            float& NewVerticalOffset
        );

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spline|Search")
        void FindClosestSplinePoint(int& SplinePointIndex);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spline|Offset")
    void GetCurrentOffsets(float& horizontal_offset, float& vertical_offset);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spline|Offset")
    void SetPlayerOffsets(float horizontal_offset, float vertical_offset);

    UFUNCTION(BlueprintCallable, Category = "Spline|Offset")
    void SetPlayerOffsetsLimits(float horizontal_limit, float vertical_limit) { horizontal_offset_limit = horizontal_limit; vertical_offset_limit = vertical_limit; };

    UFUNCTION(BlueprintCallable)
    void MovePlayerToOffsets(float horizontal_offset, float vertical_offset, float time);

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Spline|Speed")
    void SetSpeedAlongSpline(float speed);

private:
    void UpdateOffsetInterpolation();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnablePlayerControl = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float vertical_offset_limit = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float horizontal_offset_limit = 1000.f;

private:
    FTimerHandle timer_handle_offset_interp;

    float offset_interp_elapsed = 0.0f;
    float offset_interp_duration = 1.0f;
    float offset_interp_start_h = 0.0f;
    float offset_interp_start_v = 0.0f;
    float offset_interp_target_h = 0.0f;
    float offset_interp_target_v = 0.0f;
};
