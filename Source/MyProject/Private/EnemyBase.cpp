#include "EnemyBase.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"


AEnemyBase::AEnemyBase()
{
    PrimaryActorTick.bCanEverTick = true;

    auto mesh_comp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
    RootComponent = mesh_comp;

    SetActorEnableCollision(true);
    GetCapsuleComponent()->SetupAttachment(RootComponent);

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

    float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
    GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &AEnemyBase::Attack, TimeBetweenShots, true, FirstDelay);
}

void AEnemyBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateAI(DeltaTime);
}

void AEnemyBase::UpdateAI(float DeltaTime)
{
    if (bIsDead)
    {
        return;
    }

    if (Target == nullptr)
    {
        // Find the player character as target
        Target = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    }

    if (Target == nullptr)
    {
        return;
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector SelfLocation = GetActorLocation();
    FVector Direction = TargetLocation - SelfLocation;
    //Direction.Z = 0.0f;
    Direction.Normalize();

    // Move towards the target
    auto CharMovement = GetCharacterMovement();
    if (CharMovement != nullptr)
    {
        //CharMovement->AddInputVector(Direction * MoveSpeed * DeltaTime);
    }

    // Rotate towards the target
    FRotator LookAtRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
    RootComponent->SetWorldRotation(LookAtRotation);

    // Check if the enemy is in range to attack
    float DistanceToTarget = FVector::Distance(TargetLocation, SelfLocation);
    if (DistanceToTarget <= AttackRange)
    {
        // Attack the target
        Attack();
    }
}

void AEnemyBase::Attack()
{
    LastFireTime = GetWorld()->TimeSeconds;

    if (ProjectileClass != nullptr)
    {
        // Spawn projectile at the enemy's location
        FVector SpawnLocation = GetActorLocation();
        FRotator SpawnRotation = RootComponent->GetComponentRotation();
        AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(ProjectileClass, SpawnLocation, SpawnRotation);

        if (Projectile != nullptr)
        {
            // Set the projectile's owner to this enemy
            Projectile->SetOwner(this);
            Projectile->SetProjectileTrajectory(GetActorForwardVector() * 1000.f);
            Projectile->SetProjectileCollision(TEXT("EnemyProjectile"));
            Projectile->SetSpawnLocation(GetActorLocation());
            Projectile->SetProjectileMaxDistance(10000.f);
        }
    }
}

void AEnemyBase::TakeDamageImpl(float Damage)
{
    if (bIsDead)
    {
        return;
    }

    Health -= Damage;

    if (Health <= 0.0f)
    {
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