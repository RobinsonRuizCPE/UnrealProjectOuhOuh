#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "ProjectileBase.h"
#include "BehaviorTree/BehaviorTree.h"

#include "EnemyBase.generated.h"

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

protected:
    void UpdateAI(float DeltaTime);

public:
    //UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    //    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
        float MoveSpeed = 200.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
        float AttackRange = 500.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
        float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
        TSubclassOf<AProjectileBase> ProjectileClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
        UAnimationAsset* DeathAnimation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
        float DestroyDelay = 2.0f;

protected:
    void Attack();

public:
    void TakeDamageImpl(float Damage);

protected:
    void Die();

    void DestroyEnemy();

public:
    float GetHealth() const;

    float GetMaxHealth() const;

    bool IsDead() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
        float Health;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
        bool bIsDead = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
        class ACharacter* Target;

private:
    float LastFireTime;
    float TimeBetweenShots = 2.f;
    FTimerHandle TimerHandle_TimeBetweenShots;
};