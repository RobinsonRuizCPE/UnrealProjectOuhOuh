#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "SynthwaveCloud.generated.h"

UCLASS()
class MYPROJECT_API ASynthwaveCloud : public AActor
{
    GENERATED_BODY()
	
public:	
    ASynthwaveCloud();

protected:
    virtual void BeginPlay() override;

public:	
    virtual void Tick(float DeltaTime) override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud")
        UMaterialInterface* CloudMaterial;

private:
    UPROPERTY(VisibleAnywhere)
    UProceduralMeshComponent* cloud_mesh;

    void generate_low_poly_cloud();

public:
    UPROPERTY(EditAnywhere, Category = "Cloud")
        FVector movement_direction = FVector(1, 0, 0); // left to right

    UPROPERTY(EditAnywhere, Category = "Cloud")
        float move_speed = 20.f;

    UPROPERTY(EditAnywhere, Category = "Cloud")
        FRotator rotation_speed = FRotator(0.f, 5.f, 0.f);

    UPROPERTY(EditAnywhere, Category = "Cloud")
        float lifetime_seconds = 30.f;

    float lifetime_timer = 0.f;
};
