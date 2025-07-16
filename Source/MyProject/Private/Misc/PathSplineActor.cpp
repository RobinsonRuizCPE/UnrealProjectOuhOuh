// Fill out your copyright notice in the Description page of Project Settings.


#include "Misc/PathSplineActor.h"

#include "Components/SceneComponent.h"
#include "LandscapeSplineControlPoint.h"


// Constructor
APathSplineActor::APathSplineActor() : Super()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
    SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("Path Spline"));
}


FVector APathSplineActor::GetTangentPoint(TObjectPtr<ULandscapeSplineControlPoint> LandscapePoint, TObjectPtr<ULandscapeSplineSegment> LandscapeSegment, ETangentType TangentType)
{
    auto tanLen = LandscapeSegment->Connections[TangentType == ETangentType::ArriveTangent ? 1 : 0].TangentLen;
    FVector worldSpaceTanNormalized = LandscapePoint->Rotation.RotateVector({ 1.f, 0.f, 0.f });
    return worldSpaceTanNormalized * tanLen * (TangentType == ETangentType::ArriveTangent ? -1 : 1);
}

// Pull the landscape spline from the level and copy it to the spline component.  Will clear the existing splinecomponent!
void APathSplineActor::GenerateSpline()
{
#if WITH_EDITOR

    if (Landscape != nullptr) {
        // Erase pre-existing spline points
        SplineComponent->ClearSplinePoints(true);

        // Get the spline data from the landscape.
        auto LandscapeSpline = Landscape->GetSplinesComponent();


        // Note:  ControlPoints is protected.  You will need to edit LandscapeSplinesComponent.h to make it public.
        auto LandscapeSplinePoints = LandscapeSpline->GetControlPoints();
        auto LandscapeSplineSegments = LandscapeSpline->GetSegments();

        int maxIndex = LandscapeSplinePoints.Num() - 1;
        SplineComponent->AddPoint(
            FSplinePoint(0.f
                , LandscapeSplinePoints[0]->Location + Landscape->GetActorLocation()
                , FVector{ 0.f }
                , GetTangentPoint(LandscapeSplinePoints[0], LandscapeSplineSegments[0], ETangentType::LeaveTangent)
                , LandscapeSplinePoints[0]->Rotation
            ), false);


        for (int i = 1; i < maxIndex; i++) {
            SplineComponent->AddPoint(
                FSplinePoint(i
                    , LandscapeSplinePoints[i]->Location + Landscape->GetActorLocation()
                    , GetTangentPoint(LandscapeSplinePoints[i], LandscapeSplineSegments[i - 1], ETangentType::ArriveTangent)
                    , GetTangentPoint(LandscapeSplinePoints[i], LandscapeSplineSegments[i], ETangentType::LeaveTangent)
                    , LandscapeSplinePoints[i]->Rotation
                ), false);
        }

        SplineComponent->AddPoint(
            FSplinePoint(maxIndex
                , LandscapeSplinePoints[maxIndex]->Location + Landscape->GetActorLocation()
                , GetTangentPoint(LandscapeSplinePoints[maxIndex], LandscapeSplineSegments[maxIndex - 1], ETangentType::ArriveTangent)
                , FVector{ 0.f }
                , LandscapeSplinePoints[maxIndex]->Rotation
            ), false);
        SplineComponent->UpdateSpline();
    }
#endif
}
