#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DestroyableObstacle.generated.h"

class UStaticMeshComponent;
class UGeometryCollectionComponent;
class UNiagaraSystem;
class USoundBase;
class UFieldSystemComponent;

UCLASS()
class MYPROJECT_API ADestroyableObstacle : public AActor
{
    GENERATED_BODY()

public:
    ADestroyableObstacle();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

protected:
    UFUNCTION()
        void OnMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    UFUNCTION()
        void OnMeshOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // Only callable cause UE 5 blueprint hot reload does not exists...
    UFUNCTION(BlueprintCallable)
    void OnGeoCollectionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    void HandleDestruction(FVector impact_location = FVector::ZeroVector);

    void OptimizePostDestruction();

protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
        UStaticMeshComponent* RootMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
        UGeometryCollectionComponent* GeometryCollection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
        UNiagaraSystem* DestructionEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
        USoundBase* DestructionSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
        float DamageToDeal = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
        float DelayBeforeOptimize = 7.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chaos")
        UFieldSystemComponent* FieldSystem;

private:
    bool bHasBeenDestroyed = false;
};
