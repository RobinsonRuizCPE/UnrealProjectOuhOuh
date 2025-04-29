// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "EnemyDrone.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT_API AEnemyDrone : public AEnemyBase
{
	GENERATED_BODY()

public:
    AEnemyDrone();

    virtual void Tick(float DeltaTime) override;

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

private:
    void SpawnAntennaDebris(FTransform const& bone_transform, const FVector& Impulse);

protected:
    /** Per-antenna health values */
    UPROPERTY(EditDefaultsOnly, Category = "Drone|Damage")
        TMap<FName, float> AntennaHealth;

    /** How much damage the drone takes when an antenna is destroyed */
    UPROPERTY(EditDefaultsOnly, Category = "Drone|Damage")
        float AntennaDestructionDamage = 25.0f;

private:
    UPROPERTY(EditDefaultsOnly, Category = "Drone|Debris")
        UStaticMesh* AntennaDebrisMesh;

};
