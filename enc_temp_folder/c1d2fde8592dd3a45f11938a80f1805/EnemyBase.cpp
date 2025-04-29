#include "EnemyBase.h"
#include "VanquishCharacter.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"


#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"


AEnemyBase::AEnemyBase()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = GetCapsuleComponent();

    SetActorEnableCollision(true);
    //GetCapsuleComponent()->SetupAttachment(RootComponent);
}

void AEnemyBase::BeginPlay()
{
    Super::BeginPlay();
    Health = MaxHealth;

    //if (BehaviorTree != nullptr)
    {
        //RunBehaviorTree(BehaviorTree);
    }
    
    GetMesh()->SetCollisionProfileName(TEXT("Enemy"));
    UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(GetMesh()->GetOverlayMaterial(), nullptr);
    GetMesh()->SetOverlayMaterial(MaterialInstance);
    MaterialInstance->SetScalarParameterValue(FName{ "HitEffectStrength" }, 0.0);
    SetActorEnableCollision(true);

    float FirstDelay = FMath::Max(LastFireTime + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);
    GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, this, &AEnemyBase::Attack, TimeBetweenShots, true, FirstDelay);

    GetCharacterMovement()->bOrientRotationToMovement = false;
    bUseControllerRotationYaw = false;
}

void AEnemyBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bIsDead) {
        return;
    }

    if (Target == nullptr) {
        Target = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    }
}

void AEnemyBase::UpdateFollowPlayer(float DeltaTime)
{
    auto const target_player = Cast<AVanquishCharacter>(Target);
    if (!target_player)
    {
        return;
    }

    FTransform player_spline_transform = target_player->GetCurrentTransformAlongSpline();
    FVector desired_location = player_spline_transform.GetLocation() + player_spline_transform.GetRotation().GetForwardVector() * FollowDistance;
    MoveTowards(desired_location, DeltaTime);
    float distance = FVector::Dist(GetActorLocation(), desired_location);
    if (distance < 20.f) // Only move if noticeably off
    {
        GetCharacterMovement()->Velocity = FVector::ZeroVector;
        DecideNextAction();
    }
}

void AEnemyBase::UpdateMoveRandomAdjacent(float delta_time)
{
    MoveTowards(NextTargetLocation, delta_time);

    float distance = FVector::Dist(GetActorLocation(), NextTargetLocation);
    if (distance < 20)
    {
        CurrentState = EEnemyState::Attack;
    }
}

void AEnemyBase::UpdateRotation() {
    FVector TargetLocation = Target->GetActorLocation();
    FVector SelfLocation = GetActorLocation();
    FVector Direction = (TargetLocation - SelfLocation).GetSafeNormal();

    // Rotate towards the target
    FRotator LookAtRotation = FRotationMatrix::MakeFromY(Direction).Rotator();
    RootComponent->SetWorldRotation(Direction.ToOrientationRotator());
}

void AEnemyBase::MoveTowards(FVector target, float delta_time) {
    FVector direction = (target - GetActorLocation()).GetSafeNormal();
    float distance = FVector::Dist(GetActorLocation(), target);
    float speed = FMath::Clamp(distance * 5.f, 600.f, 2500.f);
    GetCharacterMovement()->Velocity = direction * speed;
}


void AEnemyBase::DecideNextAction() {
    float random_value = FMath::FRand();
    if (random_value < 0.5f) // 50% chance
    {
        PickRandomAdjacentLocation();
        CurrentState = EEnemyState::MoveRandomAdjacent;
    }
    else
    {
        CurrentState = EEnemyState::Attack;
    }
}

void AEnemyBase::PickRandomAdjacentLocation()
{
    auto const target_player = Cast<AVanquishCharacter>(Target);
    if (!target_player)
    {
        return;
    }

    FVector player_forward = target_player->GetActorForwardVector();
    FVector player_location = target_player->GetActorLocation();
    FVector offset_center = player_location + player_forward * FollowDistance;

    FVector2D random_offset = FVector2D(
        FMath::FRandRange(-500.f, 500.f),
        FMath::FRandRange(-500.f, 500.f)
    );

    FVector right = FVector::CrossProduct(FVector::UpVector, player_forward).GetSafeNormal();
    FVector up = FVector::UpVector;

    NextTargetLocation = offset_center + (right * random_offset.X) + (up * random_offset.Y);
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

    if (!ProjectileClass) {
        return;
    }

    // Spawn projectile at the enemy's location
    FVector SpawnLocation = GetActorLocation();
    FRotator SpawnRotation = GetActorRotation();
    AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(ProjectileClass, SpawnLocation, SpawnRotation);

    if (!Projectile) {
        return;
    }

    // Set the projectile's owner to this enemy
    Projectile->SetOwner(this);
    Projectile->SetProjectileTrajectory(SpawnRotation.Vector() * 1000.f);
    Projectile->SetProjectileCollision(TEXT("EnemyProjectile"));
    Projectile->SetSpawnLocation(GetActorLocation());
    Projectile->SetProjectileMaxDistance(10000.f);
    DecideNextAction();
}
    


float AEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    ToggleGlow(true);
    FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &AEnemyBase::ToggleGlow, false);
    GetWorldTimerManager().SetTimer(TimerHandle_TimeForHitGlow, RespawnDelegate, 2000.0f, false, false);
    if (bIsDead) {
        return DamageAmount;
    }

    Health -= DamageAmount;
    if (Health <= 0.0f)
    {
        //ToggleGlow(false);
        Die();
    }

    return DamageAmount;
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

