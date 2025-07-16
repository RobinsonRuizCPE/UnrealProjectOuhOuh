#include "Obstacles/DestroyableObstacle.h"

#include <VanquishCharacter.h>
#include <ProjectileBase.h>
#include <Player/SwordAttack/SwordSlashProjectile.h>

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
    GeometryCollection->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

    UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(RootMesh->GetOverlayMaterial(), nullptr);
    RootMesh->SetOverlayMaterial(MaterialInstance);
    MaterialInstance->SetScalarParameterValue(FName{ "HitEffectStrength" }, 0.0);
    // Apply to all mesh components in this actor
    TArray<UStaticMeshComponent*> mesh_components;
    GetComponents<UStaticMeshComponent>(mesh_components);
    for (UActorComponent* comp : mesh_components)
    {
        if (UStaticMeshComponent* mesh = Cast<UStaticMeshComponent>(comp))
        {
            mesh->SetOverlayMaterial(MaterialInstance);
        }
    }

}

void ADestroyableObstacle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ADestroyableObstacle::OnMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (SwordOnly && !Cast<ASwordSlashProjectile>(OtherActor)) {
        return;
    }

    auto* projectile = Cast<AProjectileBase>(OtherActor);
    if (!projectile || bHasBeenDestroyed) { 
        return;
    }

    
    if (HeatLevelMin) {
        auto player_pawn = Cast<AVanquishCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
        if (HeatLevelMin > player_pawn->GetHeatSystemLevel()) {
            ToggleGlow(true, FLinearColor{ 0.5, 0.5, 0.5, 0.1 });
            FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &ADestroyableObstacle::ToggleGlow, false, FLinearColor{ 0.5, 0.5, 0.5, 0.1 });
            GetWorldTimerManager().SetTimer(TimerHandle_TimeForHitGlow, RespawnDelegate, 1.f, false, 1.f);
            return;
        }
    }

    ToggleGlow(true, FLinearColor{ 1,0.17,0,0.1 });
    FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &ADestroyableObstacle::ToggleGlow, false, FLinearColor{1,0.17,0,0.1});
    GetWorldTimerManager().SetTimer(TimerHandle_TimeForHitGlow, RespawnDelegate, 1.f, false, 1.f);

    DamageToDeal -= projectile->GetProjectileDamage();
    if (DamageToDeal <= 0.0f)
    {
        HandleDestruction(Hit.ImpactPoint);
    }
}

void ADestroyableObstacle::OnMeshOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (SwordOnly && !Cast<ASwordSlashProjectile>(OtherActor)) {
        return;
    }

    auto* projectile = Cast<AProjectileBase>(OtherActor);
    if (!projectile || bHasBeenDestroyed) {
        return;
    }


    if (HeatLevelMin) {
        auto player_pawn = Cast<AVanquishCharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
        if (HeatLevelMin > player_pawn->GetHeatSystemLevel()) {
            ToggleGlow(true, FLinearColor{ 1, 1.f, 1.f, 0.3 });
            FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &ADestroyableObstacle::ToggleGlow, false, FLinearColor{ 1, 1.f, 1.f, 0.3 });
            GetWorldTimerManager().SetTimer(TimerHandle_TimeForHitGlow, RespawnDelegate, 1.f, false, 1.f);
            return;
        }
    }

    ToggleGlow(true, FLinearColor{ 1,0.17,0,0.3 });
    FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &ADestroyableObstacle::ToggleGlow, false, FLinearColor{ 1,0.17,0,0.3 });
    GetWorldTimerManager().SetTimer(TimerHandle_TimeForHitGlow, RespawnDelegate, 1.f, false, 1.f);

    DamageToDeal -= projectile->GetProjectileDamage();
    if (DamageToDeal <= 0.0f)
    {
        HandleDestruction(SweepResult.ImpactPoint);
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
        100.f,
        0.f,
        1.f,
        0.f,
        100.f * GetActorScale().X,
        hit_location,
        EFieldFalloffType::Field_FallOff_None
    );

    FieldSystem->ApplyPhysicsField(true, EFieldPhysicsType::Field_ExternalClusterStrain, nullptr, radial_field);
    FVector impulse = projectile->GetVelocity().GetSafeNormal() * projectile->GetProjectileDamage();
    if (projectile->GetVelocity().GetSafeNormal().IsZero()) {
        impulse = -Hit.ImpactNormal.GetSafeNormal() * projectile->GetProjectileDamage();
    }
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

    OnMeshDestroyed();

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
    OnDeathOptimizationStarts();
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

void ADestroyableObstacle::ToggleGlow(bool const activation, FLinearColor const color) {
    auto const overlay_instance = Cast<UMaterialInstanceDynamic>(RootMesh->GetOverlayMaterial());
    if (!overlay_instance) {
        return;
    }

    overlay_instance->SetVectorParameterValue(FName{ "EmissiveColor" }, color);
    overlay_instance->SetScalarParameterValue(FName{ "HitEffectStrength" }, activation ? 0.1f : 0.0f);
}
