// Fill out your copyright notice in the Description page of Project Settings.


#include "Landscape/SynthwaveCloudSpawner.h"

// Sets default values
ASynthwaveCloudSpawner::ASynthwaveCloudSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ASynthwaveCloudSpawner::BeginPlay()
{
    Super::BeginPlay();
    spawn_origin = GetActorLocation();

    for (auto i = 0; i < 10; ++i)
    {
        auto const initial_offset = FVector{ i * 4000.f , 0.f, FMath::FRand() * 5000.f } + FMath::VRand() * FVector { 500, 200, 200 };
        spawn_cloud(initial_offset);
    }

    GetWorldTimerManager().SetTimer(spawn_timer, [&]()
        {
            spawn_cloud(FVector::Zero());
        }, spawn_interval, true);
}

void ASynthwaveCloudSpawner::spawn_cloud(FVector const& additional_offset)
{
    if (spawned_count >= max_clouds)
        return;

    FVector spawn_loc = spawn_origin + FMath::VRand()* FVector{ 0, 200, 200 } + additional_offset;
    FTransform spawn_tf(FRotator::ZeroRotator, spawn_loc);
    ASynthwaveCloud* cloud = GetWorld()->SpawnActor<ASynthwaveCloud>(cloud_class, spawn_tf);

    if (cloud)
    {
        cloud->move_speed = FMath::FRandRange(100.f, 150.f);
        cloud->rotation_speed = FRotator(0.f, FMath::FRandRange(1.f, 5.f), 0.f);
        cloud->lifetime_seconds = 300.f;
        cloud->OnDestroyed.AddDynamic(this, &ASynthwaveCloudSpawner::handle_cloud_destroyed);
    }

    spawned_count++;
}

// Called every frame
void ASynthwaveCloudSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

