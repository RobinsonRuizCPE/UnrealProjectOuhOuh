// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileBase.h"

#include "Components/MeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "EnemyBase.h"
#include "VanquishCharacter.h"

// Sets default values
AProjectileBase::AProjectileBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    ProjectileMesh->SetVisibility(true);
    ProjectileMesh->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnOverlap);
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
    ProjectileMovementComponent->Velocity = world_direction * 10;
}

// Called when the game starts or when spawned
void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
    if(GetOwner())
    {
        ProjectileMesh->IgnoreActorWhenMoving(GetOwner(), true);
    }

    if (TraceEffect) {
        TraceEffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            TraceEffect,
            ProjectileMesh,           // Attach to the mesh so it follows properly
            NAME_None,                // Attach to socket name, none in this case
            FVector::ZeroVector,      // Relative location
            FRotator::ZeroRotator,    // Relative rotation
            EAttachLocation::SnapToTarget,
            true                      // Auto destroy with parent
        );

        TraceEffectComponent->SetupAttachment(ProjectileMesh);
        TraceEffectComponent->SetAutoActivate(false); // Don't play until we tell it
    }

    if (TraceEffect && TraceEffectComponent) {
        TraceEffectComponent->SetAsset(TraceEffect);
        TraceEffectComponent->Activate(true);
    }
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
    HandleProjectileImpact(OtherActor, Hit.ImpactPoint, Hit);
}

void AProjectileBase::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
    HandleProjectileImpact(OtherActor, SweepResult.ImpactPoint, SweepResult);
}

// Called every frame
void AProjectileBase::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
    if (ComputeTraveledDistance() >= MaxRange) {
        Destroy();
    }
}

void AProjectileBase::HandleProjectileImpact(AActor* OtherActor, FVector const& ImpactPoint, const FHitResult& HitResult)
{
    if (HitEffect) {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, ImpactPoint);
    }

    if (HitSound) {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, ImpactPoint);
    }

    if (TraceEffectComponent) {
        TraceEffectComponent->Deactivate();
    }

    UGameplayStatics::ApplyPointDamage(OtherActor, ProjectileDamage, ProjectileMovementComponent->Velocity, HitResult, GetInstigatorController(), this, UDamageType::StaticClass());

    Destroy();
}

float const AProjectileBase::ComputeTraveledDistance() {
    return FVector::Distance(SpawnLocation, GetActorLocation());
}

void AProjectileBase::SetProjectileCollision(FName const InCollisionProfileName) {
    ProjectileMesh->SetCollisionProfileName(InCollisionProfileName);
}
