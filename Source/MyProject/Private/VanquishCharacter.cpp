// Fill out your copyright notice in the Description page of Project Settings.


#include "VanquishCharacter.h"

// Sets default values
AVanquishCharacter::AVanquishCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AVanquishCharacter::BeginPlay()
{
	Super::BeginPlay();
	
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

