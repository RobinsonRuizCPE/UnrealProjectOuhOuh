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
    Attack
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
    void UpdateFollowPlayer(float DeltaTime);
    void UpdateMoveRandomAdjacent(float delta_time);
    void UpdateRotation();

    void DecideNextAction();
    void PickRandomAdjacentLocation();

private:
    void MoveTowards(FVector target, float delta_time);


public:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
        float MoveSpeed = 200.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
        float AttackRange = 700.0f;

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

    EEnemyState CurrentState = EEnemyState::FollowPlayer;

private:

    UNiagaraComponent* DeathEffectComponent;
    float LastFireTime;
    float TimeBetweenShots = 2.f;
    float FollowDistance = 600.f;
    FTimerHandle TimerHandle_TimeBetweenShots;
    FTimerHandle TimerHandle_TimeForHitGlow;
    FVector2D RelativeRandomOffset;
};