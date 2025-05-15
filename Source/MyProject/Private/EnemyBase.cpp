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
}

void AEnemyBase::BeginPlay()
{
    Super::BeginPlay();
    Health = MaxHealth;

    UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(GetMesh()->GetOverlayMaterial(), nullptr);
    GetMesh()->SetOverlayMaterial(MaterialInstance);
    MaterialInstance->SetScalarParameterValue(FName{ "HitEffectStrength" }, 0.0);
    SetActorEnableCollision(true);

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

void AEnemyBase::UpdateFollowPlayer(float delta_time) {
    auto const target_player = Cast<AVanquishCharacter>(Target);
    if (!target_player) return;

    FTransform player_spline_transform = target_player->GetCurrentTransformAlongSpline();
    FVector player_location = player_spline_transform.GetLocation();
    FVector player_forward = player_spline_transform.GetRotation().GetForwardVector();

    // Call common velocity function with no lateral offset
    FVector velocity = ComputeAxisSeparatedOffsetVelocity(GetActorLocation(), player_location, player_forward, FVector2D(0.f, 0.f), FollowDistance, ForwardCorrectionSpeed, 0.f);

    GetCharacterMovement()->Velocity = velocity;

    // Optional: use forward-only distance check
    FVector center_target = player_location + player_forward * FollowDistance;
    float distance = FVector::Dist(GetActorLocation(), center_target);

    if (distance < 200.f)
    {
        GetCharacterMovement()->Velocity = FVector::ZeroVector;
        DecideNextAction();
    }
}

void AEnemyBase::UpdateMoveRandomAdjacent(float delta_time) {
    auto const target_player = Cast<AVanquishCharacter>(Target);
    if (!target_player) return;

    FTransform player_spline_transform = target_player->GetCurrentTransformAlongSpline();
    FVector player_location = player_spline_transform.GetLocation();
    FVector player_forward = player_spline_transform.GetRotation().GetForwardVector();

    FVector velocity = ComputeAxisSeparatedOffsetVelocity(GetActorLocation(), player_location, player_forward, RelativeRandomOffset, FollowDistance, ForwardCorrectionSpeed, OffsetMovementSpeed);

    GetCharacterMovement()->Velocity = velocity;

    // Distance to lateral offset only (ignoring forward)
    FVector center_target = player_location + player_forward * FollowDistance;
    FVector player_right = FVector::CrossProduct(FVector::UpVector, player_forward).GetSafeNormal();
    FVector player_up = FVector::UpVector;
    FVector offset_target = center_target + (player_right * RelativeRandomOffset.X) + (player_up * RelativeRandomOffset.Y);

    float lateral_distance = FVector::DistXY(GetActorLocation(), offset_target);
    if (lateral_distance < 200.f)
    {
        BeginWaitState(2);
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

void AEnemyBase::UpdateWaitingState(float delta_time)
{
    auto const target_player = Cast<AVanquishCharacter>(Target);
    if (!target_player)
    {
        return;
    }

    FTransform player_spline_transform = target_player->GetCurrentTransformAlongSpline();

    FVector player_location = player_spline_transform.GetLocation();
    FVector player_forward = player_spline_transform.GetRotation().GetForwardVector();
    FVector velocity = ComputeAxisSeparatedOffsetVelocity(GetActorLocation(), player_location, player_forward, RelativeRandomOffset, FollowDistance, ForwardCorrectionSpeed, OffsetMovementSpeed);

    GetCharacterMovement()->Velocity = velocity;
}

FVector const AEnemyBase::ComputeAxisSeparatedOffsetVelocity(FVector current_location, FVector player_position, FVector player_forward, FVector2D relative_offset, float follow_distance, float forward_speed_max, float offset_speed_max) const {
    // Locked position in front of player
    FVector center_target = player_position + player_forward * follow_distance;

    // Right + Up vector for local offset
    FVector player_right = FVector::CrossProduct(FVector::UpVector, player_forward).GetSafeNormal();
    FVector player_up = FVector::UpVector;
    FVector offset_target = center_target + (player_right * relative_offset.X) + (player_up * relative_offset.Y);

    // --- Forward correction ---
    FVector to_center = center_target - current_location;
    float forward_amount = FVector::DotProduct(to_center, player_forward);
    FVector forward_correction = player_forward * forward_amount;

    float forward_distance = FMath::Abs(forward_amount);
    float forward_speed = FMath::Clamp(forward_distance * 10.f, 200.f, ForwardCorrectionSpeed);
    FVector forward_velocity = forward_correction.GetSafeNormal() * forward_speed;

    // --- Lateral/vertical offset ---
    FVector offset_vector = FVector::VectorPlaneProject(offset_target - current_location, player_forward);
    float offset_distance = offset_vector.Size();
    float offset_speed = FMath::Clamp(offset_distance * 10.f, 150.f, OffsetMovementSpeed);
    FVector offset_velocity = offset_vector.GetSafeNormal() * offset_speed;

    return forward_velocity + offset_velocity;
}


void AEnemyBase::DecideNextAction() {
    float random_value = FMath::FRand();
    if (random_value < 0.5f) // 50% chance
    {
        PickRandomAdjacentLocation();
        mCurrentState = EEnemyState::MoveRandomAdjacent;
    }
    else
    {
        mCurrentState = EEnemyState::Attack;
    }
}

void AEnemyBase::PickRandomAdjacentLocation()
{
    FVector2D new_offset = RelativeRandomOffset;
    const int max_attempts = 10;
    auto const min_offset = 200;
    int attempts = 0;

    while (FVector2D::Distance(new_offset, RelativeRandomOffset) < min_offset && attempts < max_attempts) {
        new_offset = FVector2D(FMath::FRandRange(-800.f, 800.f), FMath::FRandRange(-700.f, 700.f));
        attempts++;
    }

    RelativeRandomOffset = new_offset;
}

void AEnemyBase::Attack()
{
    if (!Target || bIsDead)
        return;

    float distance = FVector::Distance(Target->GetActorLocation(), GetActorLocation());
    if (distance > AttackRange) {
        BeginWaitState(0.5f);
        return;
    }

    UpdateRotation();

    PerformAttack();
    BeginWaitState(GetAttackWaitTime());
}

// BASE BEhavior but might delete this later
void AEnemyBase::PerformAttack()
{
    if (!ProjectileClass)
        return;

    FVector spawn_location = GetActorLocation();
    FRotator spawn_rotation = GetActorRotation();

    AProjectileBase* projectile = GetWorld()->SpawnActor<AProjectileBase>(ProjectileClass, spawn_location, spawn_rotation);
    if (!projectile)
        return;

    projectile->SetOwner(this);
    projectile->SetProjectileTrajectory(spawn_rotation.Vector() * 1000.f);
    projectile->SetProjectileCollision(TEXT("EnemyProjectile"));
    projectile->SetSpawnLocation(spawn_location);
    projectile->SetProjectileMaxDistance(10000.f);
}

void AEnemyBase::BeginWaitState(float duration) {
    mCurrentState = EEnemyState::Waiting;

    // Set a timer to call FinishWaitState after 'duration'
    FTimerDelegate TimerCallback;
    TimerCallback.BindUObject(this, &AEnemyBase::FinishWaitState);

    GetWorldTimerManager().SetTimer(WaitTimerHandle, TimerCallback, duration, false);
}

void AEnemyBase::FinishWaitState() {
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

