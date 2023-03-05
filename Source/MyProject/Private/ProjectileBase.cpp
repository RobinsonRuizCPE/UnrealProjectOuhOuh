// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileBase.h"

#include "Components/MeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "EnemyBase.h"

// Sets default values
AProjectileBase::AProjectileBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    ProjectileMesh->SetVisibility(true);
    ProjectileMesh->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
	SetRootComponent(ProjectileMesh);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->InitialSpeed = 3000.0f;
    ProjectileMovementComponent->MaxSpeed = 3000.0f;
    ProjectileMovementComponent->bInitialVelocityInLocalSpace = false;
    ProjectileMovementComponent->bRotationFollowsVelocity = false;
    ProjectileMovementComponent->bShouldBounce = false;
    ProjectileMovementComponent->Bounciness = 0.0f;
    ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
}

void AProjectileBase::SetProjectileTrajectory(FVector const& world_direction)
{
    ProjectileMovementComponent->Velocity = world_direction;
}

// Called when the game starts or when spawned
void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (HitEffect) {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, Hit.Location);
    }

    if (HitSound) {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, Hit.Location);
    }

    Destroy();

    if (OtherActor) {
        if (OtherActor->GetClass()->IsChildOf(AEnemyBase::StaticClass())) {
            Cast<AEnemyBase>(OtherActor)->TakeDamageImpl(ProjectileDamage);
        }
    }
}

// Called every frame
void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    if (ComputeTraveledDistance() >= MaxRange)
    {
        Destroy();
    }
}

float const AProjectileBase::ComputeTraveledDistance() {
    return FVector::Distance(SpawnLocation, GetActorLocation());
}

void AProjectileBase::SetProjectileCollision(FName const InCollisionProfileName) {
    ProjectileMesh->SetCollisionProfileName(InCollisionProfileName);
}
