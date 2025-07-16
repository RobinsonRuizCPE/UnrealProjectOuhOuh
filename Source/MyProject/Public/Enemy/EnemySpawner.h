// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"
#include "../EnemyBase.h"
#include "EnemySpawner.generated.h"

UCLASS()
class MYPROJECT_API AEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemySpawner();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
		void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);

private:
	void SpawnEnemyAtLocation();

	void SpawnNextEnemy();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
		TSubclassOf<AEnemyBase> EnemyType;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Spawn")
		UShapeComponent* CollisionSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0.00001", AllowPrivateAccess = "true"))
		float CollisionRadius = 500.0f;

	/** World-space markers for laser path */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TArray<UArrowComponent*> SpawnLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool InfiniteSpawn = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float SpawnDelay = 0.0f;

private:
	int32 current_spawn_index = 0;
	FTimerHandle spawn_timer_handle;

};
