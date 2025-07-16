// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/EnemySpawner.h"
#include "Components/ActorComponent.h"

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

	auto first_spawn_point = CreateDefaultSubobject<UArrowComponent>(TEXT("SpawnLocation_Base"));
	first_spawn_point->SetupAttachment(RootComponent); // or whichever component is appropriate
	first_spawn_point->SetRelativeLocation(FVector::ZeroVector); // Optional: give it a default offset
	SpawnLocation.Add(first_spawn_point);
}

// Called when the game starts or when spawned
void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	TInlineComponentArray<UArrowComponent*> arrow_components(this);
	SpawnLocation = arrow_components; // Copy to your array
	CollisionSphere->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
}

// Called every frame
void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemySpawner::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
	SpawnEnemyAtLocation();
}

void AEnemySpawner::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	SpawnEnemyAtLocation();
}

void AEnemySpawner::SpawnEnemyAtLocation()
{
    if (!EnemyType || !SpawnLocation.Num())
    {
        return;
    }

    UWorld* world = GetWorld();
    if (!world)
    {
        return;
    }

	current_spawn_index = 0;
	SpawnNextEnemy();
}

void AEnemySpawner::SpawnNextEnemy() {
	if (current_spawn_index >= SpawnLocation.Num())
	{
		if (InfiniteSpawn) { current_spawn_index = 0; }
		else { Destroy(); return; }
	}

	UWorld* world = GetWorld();
	if (!world || !EnemyType) return;

	USceneComponent* spawn_point = SpawnLocation[current_spawn_index];
	if (spawn_point)
	{
		FVector spawn_position = spawn_point->GetComponentLocation();
		FRotator spawn_rotation = spawn_point->GetComponentRotation();

		FActorSpawnParameters spawn_params;
		spawn_params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		world->SpawnActor<AEnemyBase>(EnemyType, spawn_position, spawn_rotation, spawn_params);
	}

	++current_spawn_index;

	if (current_spawn_index < SpawnLocation.Num())
	{
		world->GetTimerManager().SetTimer(spawn_timer_handle, this, &AEnemySpawner::SpawnNextEnemy, SpawnDelay, false);
	}
}


