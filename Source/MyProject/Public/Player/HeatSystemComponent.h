// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "HeatSystemComponent.generated.h"

UENUM(BlueprintType)
enum class EHeatDecayType : uint8 {
	None,
	Linear,
	Exponential,
	Logarithmic,
	QuadraticEaseIn,
	QuadraticEaseOut,
	SmoothStep,
	StepDrop,
	PulseDrop,
	SineWave,
	CurveDriven
};

USTRUCT()
struct FHeatModifier {
	GENERATED_BODY()

	float InitialHeat;
	float Duration;
	float Elapsed;
	EHeatDecayType DecayType;
	bool bIsIncrease;

	mutable float LastValue = 0.f;

	FHeatModifier()
		: InitialHeat(0), Duration(0), Elapsed(0), DecayType(EHeatDecayType::None), bIsIncrease(true) {}

	float ComputeCurrentContribution() const;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHeatLevelChanged, int32, NewHeatLevel, bool, IsIncrease);
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYPROJECT_API UHeatSystemComponent : public UActorComponent {
	GENERATED_BODY()

public:
	UHeatSystemComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Adds a heat effect (increase or decrease) with decay type over time
	UFUNCTION(BlueprintCallable)
	void AddHeatModifier(float amount, float duration, EHeatDecayType decay_type, bool b_increase);

	// Instantly changes heat without decay
	UFUNCTION(BlueprintCallable)
	void ApplyInstantHeatChange(float amount, bool b_increase);

	// Returns net change per second
	UFUNCTION(BlueprintCallable)
	float GetHeatChangePerSecond() const;

	// Current heat level (can go negative if allowed)
	float GetCurrentHeat() const { return current_heat; }

	UFUNCTION(BlueprintCallable)
	float GetCurrentHeatThresholdPercent() const;

	// Get current heat level tier
	int32 GetGlobalHeatLevel() const { return global_heat_level; }

	void DecreaseGlobalHeatLevel();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHeatLevelChanged OnHeatLevelChanged;
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
		USoundBase* HeatIncreaseSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
		USoundBase* HeatDecreaseSound;

private:
	UPROPERTY()
		TArray<FHeatModifier> active_modifiers;

	UPROPERTY(EditAnywhere, Category = "Heat")
		float current_heat = 0.f;

	UPROPERTY(EditAnywhere, Category = "Heat")
		TArray<float> heat_thresholds;

	int32 global_heat_level = 0;

	void UpdateHeatLevel();
};