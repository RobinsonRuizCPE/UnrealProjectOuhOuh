// Fill out your copyright notice in the Description page of Project Settings.


#include "VanquishCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AVanquishCharacter::AVanquishCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	mCurrentHealth = mMaxHealth;
}

// Called when the game starts or when spawned
void AVanquishCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetMesh()->SetCollisionProfileName(TEXT("Character"));
}

// Called every frame
void AVanquishCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AVanquishCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AVanquishCharacter::StartDodging() {
	b_is_dodging = true;
}

void AVanquishCharacter::EndDodging() {
	b_is_dodging = false;
}

void AVanquishCharacter::StartSwordAttack(SwordAttackType const attack_to_start) {
	b_is_attacking = true;
	e_current_sword_attack = attack_to_start;
}

void AVanquishCharacter::EndSwordAttack() {
	b_is_attacking = false;
	e_current_sword_attack = SwordAttackNone;
}

void AVanquishCharacter::TakeDamageImpl(float Damage)
{
	mCurrentHealth -= Damage;
	if (mCurrentHealth <= 0.0f)
	{
		//Die();
	}
}

void AVanquishCharacter::Die() {
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
	//GetMesh()->PlayAnimation(DeathAnimation, false);
	//
	//FTimerHandle TimerHandle;
	//GetWorldTimerManager().SetTimer(TimerHandle, this, &AEnemyBase::DestroyEnemy, DestroyDelay);
	//GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}
