// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/HeatSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UHeatSystemComponent::UHeatSystemComponent() {
	PrimaryComponentTick.bCanEverTick = true;
	heat_thresholds = { 0.f, 200.f, 500.f, 1000.f };  // Example thresholds
}

void UHeatSystemComponent::BeginPlay() {
	Super::BeginPlay();
}

void UHeatSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	float delta_heat = 0.f;

	for (int32 i = active_modifiers.Num() - 1; i >= 0; --i) {
		FHeatModifier& mod = active_modifiers[i];
		mod.Elapsed += DeltaTime;

		float current = mod.ComputeCurrentContribution();
		float delta = current - mod.LastValue;
		mod.LastValue = current;

		delta_heat += mod.bIsIncrease ? delta : -delta;

		if (mod.Elapsed >= mod.Duration) {
			active_modifiers.RemoveAt(i);
		}
	}

	current_heat += delta_heat;
	current_heat = FMath::Max(0.f, current_heat); // Clamp if needed

	UpdateHeatLevel();
}

void UHeatSystemComponent::AddHeatModifier(float amount, float duration, EHeatDecayType decay_type, bool b_increase) {
	FHeatModifier mod;
	mod.InitialHeat = amount;
	mod.Duration = duration;
	mod.Elapsed = 0.f;
	mod.DecayType = decay_type;
	mod.bIsIncrease = b_increase;
	active_modifiers.Add(mod);
}

void UHeatSystemComponent::ApplyInstantHeatChange(float amount, bool b_increase) {
	current_heat += b_increase ? amount : -amount;
	current_heat = FMath::Max(0.f, current_heat);
	UpdateHeatLevel();
}

float UHeatSystemComponent::GetHeatChangePerSecond() const {
	float total = 0.f;
	for (const FHeatModifier& mod : active_modifiers) {
		total += mod.bIsIncrease ? mod.ComputeCurrentContribution() : -mod.ComputeCurrentContribution();
	}
	return total;
}

float UHeatSystemComponent::GetCurrentHeatThresholdPercent() const {
	if (heat_thresholds.Num() == 0)
		return 0.f;

	float prev_threshold = heat_thresholds[global_heat_level];
	float next_threshold = (global_heat_level < heat_thresholds.Num() -1 ) ? heat_thresholds[global_heat_level + 1] : 2000.f;

	float range = next_threshold - prev_threshold;
	if (range <= 0.f)
		return 100.f;  // Already maxed

	float heat_in_range = current_heat - prev_threshold;
	return FMath::Clamp(heat_in_range / range * 100.f, 0.f, 100.f);
}

void UHeatSystemComponent::DecreaseGlobalHeatLevel() {
	active_modifiers.Empty();
	global_heat_level = FMath::Max(global_heat_level - 1, 0);
	current_heat = heat_thresholds[global_heat_level];
	OnHeatLevelChanged.Broadcast(global_heat_level, false);
	UGameplayStatics::PlaySound2D(GetWorld(), HeatDecreaseSound);
}


void UHeatSystemComponent::UpdateHeatLevel() {
	if (global_heat_level == heat_thresholds.Num() - 1) {
		return;
	}

	if (current_heat > heat_thresholds[global_heat_level + 1]) {
		active_modifiers.Empty();
		global_heat_level++;
		current_heat = heat_thresholds[global_heat_level];
		OnHeatLevelChanged.Broadcast(global_heat_level, true);
		UGameplayStatics::PlaySound2D(GetWorld(), HeatIncreaseSound);
	}
}

float FHeatModifier::ComputeCurrentContribution() const {
	if (Duration <= 0.f) return 0.f;

	float alpha = FMath::Clamp(Elapsed / Duration, 0.f, 1.f);
	switch (DecayType) {
	case EHeatDecayType::Linear:
		return InitialHeat * (1.f - alpha);

	case EHeatDecayType::Exponential:
		// Fast drop initially, slows over time
		return InitialHeat * FMath::Exp(-5.f * alpha);

	case EHeatDecayType::Logarithmic:
		// Slow at first, faster later
		return InitialHeat * (1.f - FMath::Loge(1.f + 9.f * alpha) / FMath::Loge(10.f));

	case EHeatDecayType::QuadraticEaseIn:
		// Slow start, fast end
		return InitialHeat * (1.f - FMath::Square(alpha));

	case EHeatDecayType::QuadraticEaseOut:
		// Fast start, slow end
		return InitialHeat * (1.f - FMath::Square(1.f - alpha));

	case EHeatDecayType::SmoothStep:
	{
		// S-curve (slow-fast-slow)
		float smooth = alpha * alpha * (3.f - 2.f * alpha);
		return InitialHeat * (1.f - smooth);
	}

	case EHeatDecayType::StepDrop:
		// Constant until duration ends, then 0
		return (Elapsed >= Duration) ? 0.f : InitialHeat;

	case EHeatDecayType::PulseDrop:
		// V-shape drop (centered dip)
		return InitialHeat * (1.f - FMath::Abs(0.5f - alpha) * 2.f);

	case EHeatDecayType::SineWave:
		// Oscillates while decaying
		return InitialHeat * (1.f - alpha) * FMath::Abs(FMath::Sin(alpha * PI * 4.f));

	case EHeatDecayType::None:
	default:
		return InitialHeat;
	}
}
