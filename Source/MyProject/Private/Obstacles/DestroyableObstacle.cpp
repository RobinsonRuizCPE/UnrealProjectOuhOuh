#include "Obstacles/DestroyableObstacle.h"

#include <ProjectileBase.h>

#include "Field/FieldSystem.h"
#include "Field/FieldSystemComponent.h"
#include "Field/FieldSystemObjects.h"
#include "Field/FieldSystemNodes.h"
#include "Components/StaticMeshComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"
#include "GameFramework/DamageType.h"

ADestroyableObstacle::ADestroyableObstacle()
{
    PrimaryActorTick.bCanEverTick = true;

    RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));
    RootComponent = RootMesh;
    RootMesh->SetCollisionProfileName(TEXT("DestroyableObstacle"));
    RootMesh->SetVisibility(true);
    RootMesh->SetGenerateOverlapEvents(true);
    RootMesh->OnComponentBeginOverlap.AddDynamic(this, &ADestroyableObstacle::OnMeshOverlap);
    RootMesh->OnComponentHit.AddDynamic(this, &ADestroyableObstacle::OnMeshHit);

    GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
    GeometryCollection->SetupAttachment(RootComponent);
    GeometryCollection->SetVisibility(false);
    GeometryCollection->SetSimulatePhysics(false);
    GeometryCollection->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GeometryCollection->OnComponentBeginOverlap.AddDynamic(this, &ADestroyableObstacle::OnGeoCollectionOverlap);
    GeometryCollection->OnComponentHit.AddDynamic(this, &ADestroyableObstacle::OnGeoCollectionHit);

    FieldSystem = CreateDefaultSubobject<UFieldSystemComponent>(TEXT("FieldSystem"));
    FieldSystem->SetupAttachment(RootComponent);
}

void ADestroyableObstacle::BeginPlay()
{
    Super::BeginPlay();
}

void ADestroyableObstacle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ADestroyableObstacle::OnMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!bHasBeenDestroyed)
    {
        HandleDestruction(Hit.ImpactPoint);
        UGameplayStatics::ApplyDamage(OtherActor, DamageToDeal, nullptr, this, UDamageType::StaticClass());
    }
}

void ADestroyableObstacle::OnMeshOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!bHasBeenDestroyed)
    {
        HandleDestruction(SweepResult.ImpactPoint);
        UGameplayStatics::ApplyDamage(OtherActor, DamageToDeal, nullptr, this, UDamageType::StaticClass());
    }
}

void ADestroyableObstacle::OnGeoCollectionOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    HandleGeoCollectionDamage(OtherActor, SweepResult);
}

void ADestroyableObstacle::OnGeoCollectionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    HandleGeoCollectionDamage(OtherActor, Hit);
}

void ADestroyableObstacle::HandleGeoCollectionDamage(AActor* OtherActor, const FHitResult& Hit) {
    auto* projectile = Cast<AProjectileBase>(OtherActor);
    if (!projectile || !bHasBeenDestroyed) return;

    FVector hit_location = Hit.ImpactPoint;

    // Create a radial field
    auto* radial_field = NewObject<URadialFalloff>();
    radial_field->SetRadialFalloff(
        1000.f,
        0.f,
        1.f,
        0.f,
        100.f,
        hit_location,
        EFieldFalloffType::Field_FallOff_None
    );

    FieldSystem->ApplyPhysicsField(true, EFieldPhysicsType::Field_ExternalClusterStrain, nullptr, radial_field);

    FVector impulse = projectile->GetVelocity().GetSafeNormal() * projectile->GetProjectileDamage() * 600000;
    GeometryCollection->AddImpulseAtLocation(impulse, Hit.ImpactPoint);
}

void ADestroyableObstacle::HandleDestruction(FVector impact_location)
{
    bHasBeenDestroyed = true;

    RootMesh->SetVisibility(false);
    RootMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    GeometryCollection->SetVisibility(true);
    GeometryCollection->SetSimulatePhysics(true);
    GeometryCollection->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GeometryCollection->SetNotifyRigidBodyCollision(true);
    GeometryCollection->SetGenerateOverlapEvents(true);

    if (DestructionEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), DestructionEffect, impact_location);
    }

    if (DestructionSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), DestructionSound, impact_location);
    }

    GetWorldTimerManager().SetTimerForNextTick([this]() {
        GetWorldTimerManager().SetTimerForNextTick([this]() {
            OptimizePostDestruction();
            });
        });
}

void ADestroyableObstacle::OptimizePostDestruction()
{
    FTimerHandle optimization_timer;
    GetWorldTimerManager().SetTimer(optimization_timer,
        [this]() {
            //GeometryCollection->SetSimulatePhysics(false);
            // Clever Solution -> get materials and slowly change their opactity before killing them ?
            Destroy();
        },
        DelayBeforeOptimize,
        false);
}
