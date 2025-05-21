// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "SynthwaveCloud.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SynthwaveCloudSpawner.generated.h"

UCLASS()
class MYPROJECT_API ASynthwaveCloudSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASynthwaveCloudSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void spawn_cloud(FVector const&	additional_offset = FVector::Zero());

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
	void handle_cloud_destroyed(AActor* destroyed_actor) { spawned_count = FMath::Max(0, spawned_count - 1);}

protected:
	UPROPERTY(EditAnywhere)
		TSubclassOf<ASynthwaveCloud> cloud_class;

	UPROPERTY(EditAnywhere)
		float spawn_interval = 30.f;

	UPROPERTY(EditAnywhere)
		int32 max_clouds = 20;

	UPROPERTY(EditAnywhere)
		FVector spawn_origin;

	FTimerHandle spawn_timer;
	int32 spawned_count = 0;
};
