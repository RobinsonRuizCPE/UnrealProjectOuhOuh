// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VanquishCharacterEnum.h"
#include "WeaponBase.h"
#include <Player/PlayerFollowSplineComponent.h>
#include "VanquishCharacter.generated.h"

class ASwordSlashProjectile;

UCLASS()
class MYPROJECT_API AVanquishCharacter : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVanquishCharacter();

	// Blueprint-callable methods (implemented in Blueprint)
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "VFX")
	void TriggerDodgeSlowMoVFX(bool const trigger);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void ToggleGlow(bool const activation);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/**
	Dodge Handling
	*/
	UFUNCTION(BlueprintCallable, Category=AVanquishCharacter)
	bool const GetDodgingStatus() const { return b_is_dodging; };

	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	void SetDodgingStatus(bool new_status) { b_is_dodging = new_status; }

	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	bool const GetDodgingStatusCanMoveAgain() const { return b_is_dodging_can_move_again; };

	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
		void SetDodgingStatusCanMoveAgain(bool new_status);

	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	USkeletalMeshComponent* GetMesh() { return SkeletalMesh; }

	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	FTransform const& GetCurrentTransformAlongSpline() const { return mCurrentTransformAlongSpline; };

	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	void const SetCurrentTransformAlongSpline(FTransform const current_transform) { mCurrentTransformAlongSpline = current_transform; };

	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	FVector const& GetSwordSlashCurrentPos() const { return sword_slash_current_position; };

	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	void const SetSwordSlashCurrentPos(FVector const current_pos) { sword_slash_current_position = current_pos; };

	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	void DeactivateHitboxesFor(float const seconds);

	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	void ReactivatePlayerHitboxes();
	/**
	Sword Attack Handling
	*/
	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	SwordAttackType const GetSwordAttack() const { return e_current_sword_attack; };
	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	void StartSwordAttack(SwordAttackType const attack_to_start);
	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	void EndSwordAttack();

	UFUNCTION()
	void OnComboReset();

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void TriggerSwordAttack();

	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	float const GetCurrentHealtPercentage() const { return (mCurrentHealth/mMaxHealth); };

private:
	void Die();

	float PlayAnimMontage(UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None);
	void StopAnimMontage(UAnimMontage* AnimMontage);
	UAnimMontage* GetCurrentMontage();

	void ResetGlobalTimeDilation();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* CapsuleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UPlayerFollowSplineComponent* PlayerFollowSplineComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<ASwordSlashProjectile> SwordSlashProjectileHorizontalClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float mMaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float mCurrentHealth;

	UPROPERTY(VisibleAnywhere, Category = "Sword")
	UStaticMeshComponent* SwordMesh;

private:
	// Dodge variables
	bool b_is_dodging_can_move_again = true;
	bool b_is_dodging = false;
	bool b_has_triggered_dodge_slowmo = false;
	FTimerHandle collision_restore_timer_handle;
	FTimerHandle timer_handle_reset_slowmo;

	//Attack variables
	bool b_is_attacking = false;
	bool b_attack_buffered = false;
	FVector sword_slash_current_position;
	SwordAttackType e_current_sword_attack = SwordAttackNone;
	FTimerHandle combo_reset_timer_handle;
	float combo_max_delay = 1.0f;

	FTimerHandle TimerHandle_TimeForHitGlow;
	FTransform mCurrentTransformAlongSpline;
};
