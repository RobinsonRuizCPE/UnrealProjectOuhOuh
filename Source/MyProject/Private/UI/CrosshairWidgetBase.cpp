// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CrosshairWidgetBase.h"
#include <EnemyBase.h>
#include <Obstacles/DestroyableObstacle.h>

#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"

void UCrosshairWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();
	if (BaseCrosshair)
	{
		crosshair_slot = Cast<UCanvasPanelSlot>(BaseCrosshair->Slot);
	}
}

void UCrosshairWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateCursorAccordingTargetedClass();
}

void UCrosshairWidgetBase::UpdateCursorAccordingTargetedClass() {
	if (!TargetedActorType) {
		SetCrosshairColor(FLinearColor{ 0.15f, 0.7f ,1.f, 1.f });
		BaseCrosshair->SetRenderTransformAngle(0.f);
		return;
	}

	if (TargetedActorType->IsChildOf<AEnemyBase>()) {
		SetCrosshairColor(FLinearColor{ 1.f,0,0,1.f });
		BaseCrosshair->SetRenderTransformAngle(BaseCrosshair->GetRenderTransformAngle() + 0.5);
		return;
	}

	if (TargetedActorType->IsChildOf<ADestroyableObstacle>()) {
		SetCrosshairColor(FLinearColor{ 1.f,1.f,0,1.f });
		BaseCrosshair->SetRenderTransformAngle(BaseCrosshair->GetRenderTransformAngle() + 0.5);
		return;
	}

	SetCrosshairColor(FLinearColor{ 0.15f, 0.7f ,1.f, 1.f });
	BaseCrosshair->SetRenderTransformAngle(0.f);
}

void UCrosshairWidgetBase::UpdateCrosshairPosition(FVector2D const& screen_position)
{
	if (!CrosshairOverlay)
		return;

	if (UCanvasPanelSlot* overlay_slot = Cast<UCanvasPanelSlot>(CrosshairOverlay->Slot))
	{
		overlay_slot->SetPosition(screen_position);
	}
}

void UCrosshairWidgetBase::SetCrosshairColor(FLinearColor const& new_color)
{
	if (BaseCrosshair)
	{
		BaseCrosshair->SetColorAndOpacity(new_color);
	}
}

void UCrosshairWidgetBase::UpdateChargeCrosshair(float const normalized_charge)
{
	if (!ChargeShotCrosshair)
		return;

	auto const clamped_normalized_charge = FMath::Clamp(normalized_charge, 0.0f, 1.0f);

	// Apply scaling via RenderTransform
	FWidgetTransform transform;
	transform.Scale = FVector2D(clamped_normalized_charge, clamped_normalized_charge);

	ChargeShotCrosshair->SetRenderTransform(transform);
	auto color = ChargeShotCrosshair->GetColorAndOpacity();
	color.A = clamped_normalized_charge < 1.f ? 0.5f : 1.f;
	ChargeShotCrosshair->SetColorAndOpacity(color);
}