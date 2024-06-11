#include "EnemyBase.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraParameterCollection.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"


AEnemyBase::AEnemyBase()
{
    PrimaryActorTick.bCanEverTick = true;

    RootComponent = GetCapsuleComponent();

    SetActorEnableCollision(true);
    //GetCapsuleComponent()->SetupAttachment(RootComponent);
    
    Health = MaxHealth;
}

void AEnemyBase::BeginPlay()
{
    Super::BeginPlay();
    //if (BehaviorTree != nullptr)
    {
        //RunBehaviorTree(BehaviorTree);
    }
    
    GetMesh()->SetCollisionProfileName(TEXT("Enemy"));
    UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(GetMesh()->GetOverlayMaterial(), nullptr);
    GetMesh()->SetOverlayMaterial(MaterialInstance);
    MaterialInstance->SetScalarParameterValue(FName{ "HitEffectStrength" }, 0.0);
    SetActorEnableCollision(true);

    //float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
    //GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &AEnemyBase::Attack, TimeBetweenShots, true, FirstDelay);
}

void AEnemyBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateAI(DeltaTime);
}

void AEnemyBase::UpdateAI(float DeltaTime)
{
    if (bIsDead) {
        return;
    }

    if (Target == nullptr) {
        // Find the player character as target
        Target = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    }

    if (Target == nullptr) {
        return;
    }

    UpdateRotation();
}

void AEnemyBase::UpdateRotation() {
    FVector TargetLocation = Target->GetActorLocation();
    FVector SelfLocation = GetActorLocation();
    FVector Direction = (TargetLocation - SelfLocation).GetSafeNormal();

    // Rotate towards the target
    FRotator LookAtRotation = FRotationMatrix::MakeFromY(Direction).Rotator();
    RootComponent->SetWorldRotation(Direction.ToOrientationRotator());
}

void AEnemyBase::Attack() {
    if (!Target) {
        return;
    }

    // Check if the enemy is in range to attack
    float DistanceToTarget = FVector::Distance(Target->GetActorLocation(), GetActorLocation());
    if (DistanceToTarget > AttackRange) {
        return;
    }

    //LastFireTime = GetWorld()->TimeSeconds;

    if (ProjectileClass != nullptr)
    {
        // Spawn projectile at the enemy's location
        FVector SpawnLocation = GetActorLocation();
        FRotator SpawnRotation = GetActorRotation();
        AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(ProjectileClass, SpawnLocation, SpawnRotation);

        if (Projectile != nullptr)
        {
            // Set the projectile's owner to this enemy
            Projectile->SetOwner(this);
            Projectile->SetProjectileTrajectory(SpawnRotation.Vector() * 1000.f);
            Projectile->SetProjectileCollision(TEXT("EnemyProjectile"));
            Projectile->SetSpawnLocation(GetActorLocation());
            Projectile->SetProjectileMaxDistance(10000.f);
        }
    }
}


void AEnemyBase::TakeDamageImpl(float Damage)
{
    ToggleGlow(true);
    FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &AEnemyBase::ToggleGlow, false);
    GetWorldTimerManager().SetTimer(TimerHandle_TimeForHitGlow, RespawnDelegate, 2000.0f, false, false);
    if (bIsDead) {
        return;
    }

    Health -= Damage;
    if (Health <= 0.0f)
    {
        //ToggleGlow(false);
        Die();
    }
}

void AEnemyBase::Die()
{
    bIsDead = true;

    // Disable collision
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Disable movement
    auto CharMovement = GetCharacterMovement();
    if (CharMovement != nullptr)
    {
        CharMovement->StopMovementImmediately();
    }

    // Play death animation and destroy actor after a delay
    GetMesh()->PlayAnimation(DeathAnimation, false);
    if (DeathEffect) {
        DeathEffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(DeathEffect, GetMesh(), "Core", FVector{}, FRotator{}, EAttachLocation::Type::SnapToTarget, true, true, ENCPoolMethod::None, true);
        //DeathEffectComponent->SetFloatParameter(FName("Delay"), 0.1f);
        DeathEffectComponent->SetVisibility(true, true);
    }
    

    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, this, &AEnemyBase::DestroyEnemy, DestroyDelay);
    GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void AEnemyBase::DestroyEnemy()
{
    Destroy();
}

float AEnemyBase::GetHealth() const
{
    return Health;
}

float AEnemyBase::GetMaxHealth() const
{
    return MaxHealth;
}

bool AEnemyBase::IsDead() const
{
    return bIsDead;
}


void AEnemyBase::ToggleGlow(bool const activation) {
    auto const overlay_instance = Cast<UMaterialInstanceDynamic>(GetMesh()->GetOverlayMaterial());
    overlay_instance->SetScalarParameterValue(FName{ "HitEffectStrength" }, activation ? 1.0f : 0.0f);
    GetWorldTimerManager().ClearTimer(TimerHandle_TimeForHitGlow);
}

