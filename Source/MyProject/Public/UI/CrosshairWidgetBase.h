// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Overlay.h"
#include "CrosshairWidgetBase.generated.h"

class UImage;

UCLASS()

class MYPROJECT_API UCrosshairWidgetBase : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void UpdateCrosshairPosition(FVector2D const& screen_position);

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void SetCrosshairColor(FLinearColor const& new_color);

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void SetTargetedActorClass(TSubclassOf<AActor> const& targeted_object_class) { TargetedActorType = targeted_object_class; };

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void UpdateChargeCrosshair(float normalized_charge);

protected:

	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> CrosshairOverlay;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> BaseCrosshair;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ChargeShotCrosshair;

private:
	void UpdateCursorAccordingTargetedClass();

private:

	/** Cached pointer to the slot to modify position */
	class UCanvasPanelSlot* crosshair_slot;
	TSubclassOf<AActor> TargetedActorType;
};
