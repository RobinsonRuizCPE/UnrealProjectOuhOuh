#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "ProjectileBase.h"
#include "BehaviorTree/BehaviorTree.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraParameterCollection.h"

#include "EnemyBase.generated.h"

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
    FollowPlayer,
    MoveRandomAdjacent,
    Attack,
    Waiting
};

UCLASS()
class MYPROJECT_API AEnemyBase : public ACharacter
{
    GENERATED_BODY()

public:
    AEnemyBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
    // Returns the distance from player
    void UpdateFollowPlayer(float DeltaTime);
    void UpdateMoveRandomAdjacent(float delta_time);
    void UpdateRotation();

    void UpdateWaitingState(float delta_time);

    void SetNextAction(EEnemyState const next_action) { mCurrentState = next_action; };
    void DecideNextAction();
    void PickRandomAdjacentLocation();

private:

    FVector const ComputeAxisSeparatedOffsetVelocity(FVector current_location, FVector player_position, FVector player_forward, FVector2D relative_offset, float follow_distance, float forward_speed_max, float offset_speed_max) const;


public:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
        float MoveSpeed = 200.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
        float AttackRange = 1500.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
        float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
        TSubclassOf<AProjectileBase> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
        UAnimationAsset* DeathAnimation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
        float DestroyDelay = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
        UNiagaraSystem* DeathEffect;

protected:
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void Attack();

    virtual void PerformAttack();
    virtual float GetAttackWaitTime() const { return 1.0f; }

    void BeginWaitState(float duration);

    void FinishWaitState();

protected:
    void Die();

    void DestroyEnemy();

public:
    float GetHealth() const;

    float GetMaxHealth() const;

    bool IsDead() const;

    void ToggleGlow(bool const activation);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
        float Health;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
        bool bIsDead = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
        class APawn* Target;

    EEnemyState mCurrentState = EEnemyState::FollowPlayer;

    FTimerHandle WaitTimerHandle;

private:

    bool IsFiring = false;

    UNiagaraComponent* DeathEffectComponent;
    float FollowDistance = 700.f;
    float ForwardCorrectionSpeed = 1500.f;
    float OffsetMovementSpeed = 400.f;
    FTimerHandle TimerHandle_TimeBetweenShots;
    FTimerHandle TimerHandle_TimeForHitGlow;
    FVector2D RelativeRandomOffset;
};