// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/EnemySpawner.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
AEnemySpawner::AEnemySpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetVisibility(true);
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemySpawner::OnOverlap);
	CollisionSphere->OnComponentHit.AddDynamic(this, &AEnemySpawner::OnHit);
	CollisionSphere->SetCollisionProfileName("EnemyProjectile");
	SetRootComponent(CollisionSphere);

	SpawnLocation = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnLocation"));
	SpawnLocation->SetupAttachment(RootComponent); // or whichever component is appropriate
	SpawnLocation->SetRelativeLocation(FVector::ZeroVector); // Optional: give it a default offset
}

// Called when the game starts or when spawned
void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemySpawner::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
	SpawnEnemyAtLocation();
	Destroy();
}

void AEnemySpawner::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	SpawnEnemyAtLocation();
	Destroy();
}

void AEnemySpawner::SpawnEnemyAtLocation()
{
    if (!EnemyType || !SpawnLocation)
    {
        return;
    }

    UWorld* world = GetWorld();
    if (!world)
    {
        return;
    }

    FVector spawn_position = SpawnLocation->GetComponentLocation();
    FRotator spawn_rotation = SpawnLocation->GetComponentRotation();

    FActorSpawnParameters spawn_params;
    spawn_params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	world->SpawnActor<AEnemyBase>(EnemyType, spawn_position, spawn_rotation, spawn_params);
}

