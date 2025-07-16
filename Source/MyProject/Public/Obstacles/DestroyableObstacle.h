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

    UFUNCTION(BlueprintCallable)
    void OnGeoCollectionOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // Only callable cause UE 5 blueprint hot reload does not exists...
    UFUNCTION(BlueprintCallable)
    void OnGeoCollectionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    void HandleGeoCollectionDamage(AActor* OtherActor, const FHitResult& Hit);

    void HandleDestruction(FVector impact_location = FVector::ZeroVector);

    UFUNCTION(BlueprintImplementableEvent, Category = "Obstacle")
    void OnMeshDestroyed();

    void OptimizePostDestruction();

    UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Death")
    void OnDeathOptimizationStarts();

private:
    void ToggleGlow(bool const activation, FLinearColor const color);


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
        float DamageToDeal = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
        float DelayBeforeOptimize = 7.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chaos")
        UFieldSystemComponent* FieldSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
        bool SwordOnly = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
        uint8 HeatLevelMin = 0;

private:
    bool bHasBeenDestroyed = false;
    FTimerHandle TimerHandle_TimeForHitGlow;
};
