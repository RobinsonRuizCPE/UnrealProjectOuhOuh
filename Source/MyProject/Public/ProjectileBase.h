// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "ProjectileBase.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class USoundBase;

UCLASS()
class MYPROJECT_API AProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectileBase();

	void SetProjectileTrajectory(FVector const& world_direction);
	void SetProjectileMaxDistance(float const max_distance) { MaxRange = max_distance; }
	void SetSpawnLocation(FVector3d const spawn_location) { SpawnLocation = spawn_location; }
	void SetProjectileCollision(FName const InCollisionProfileName);

private: 
	float const ComputeTraveledDistance();

	void HandleProjectileImpact(AActor* OtherActor, FVector const& ImpactPoint, const FHitResult& HitResult);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(Category = "test", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UMeshComponent* ProjectileMesh;

	UPROPERTY(Category = "Movement", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr <UProjectileMovementComponent> ProjectileMovementComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* HitEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
	UNiagaraSystem* TraceEffect;

	/** single fire sound (bLoopedFireSound not set) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	USoundBase* HitSound;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	float MaxRange = 0;
	FVector3d SpawnLocation;
	float ProjectileDamage = 10;

	UPROPERTY()
	UNiagaraComponent* TraceEffectComponent;

};
