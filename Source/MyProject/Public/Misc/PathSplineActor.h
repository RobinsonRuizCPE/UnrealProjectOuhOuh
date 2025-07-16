#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "LandscapeSplineControlPoint.h"
#include "LandscapeSplineSegment.h"
#include "LandscapeProxy.h"

#include "PathSplineActor.generated.h"

UENUM(BlueprintType)
enum class ETangentType : uint8 {
	ArriveTangent,
	LeaveTangent
};

UCLASS()
class APathSplineActor : public AActor
{
    GENERATED_BODY()


public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
        class USplineComponent* SplineComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
        ALandscapeProxy* Landscape;

    APathSplineActor();

    FVector GetTangentPoint(TObjectPtr<ULandscapeSplineControlPoint> LandscapePoint, TObjectPtr<ULandscapeSplineSegment> LandscapeSegment, ETangentType TangentType);


private:
    UFUNCTION(CallInEditor, Category = "Landscape")
        void GenerateSpline();
};