// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VanquishCharacterEnum.h"
#include "WeaponBase.h"
#include "VanquishCharacter.generated.h"


UCLASS()
class MYPROJECT_API AVanquishCharacter : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVanquishCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/**
	Dodge Handling
	*/
	UFUNCTION(BlueprintCallable, Category=AVanquishCharacter)
	bool const IsDodging() const { return b_is_dodging; };

	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	void StartDodging();

	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	void EndDodging();

	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	FTransform const& GetCurrentTransformAlongSpline() const { return mCurrentTransformAlongSpline; };

	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	void const SetCurrentTransformAlongSpline(FTransform const current_transform) { mCurrentTransformAlongSpline = current_transform; };

	/**
	Sword Attack Handling
	*/
	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	bool const IsSwordAttacking() const { return b_is_attacking; };
	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	void StartSwordAttack(SwordAttackType const attack_to_start);
	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	void EndSwordAttack();

	UFUNCTION(BlueprintCallable, Category = AVanquishCharacter)
	float const GetCurrentHealtPercentage() const { return (mCurrentHealth/mMaxHealth); };

private:
	void Die();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float mMaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float mCurrentHealth;

private:
	// Dodge variables
	bool b_is_dodging = false;

	//Attack variables
	bool b_is_attacking = false;
	SwordAttackType e_current_sword_attack = SwordAttackNone;

	FTransform mCurrentTransformAlongSpline;
};
