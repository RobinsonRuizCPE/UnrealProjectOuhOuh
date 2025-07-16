// Fill out your copyright notice in the Description page of Project Settings.


#include "VanquishCharacter.h"

#include <Player/SwordAttack/SwordSlashProjectile.h>
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AVanquishCharacter::AVanquishCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	mCurrentHealth = mMaxHealth;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->InitCapsuleSize(25.f, 25.f);
	SetRootComponent(CapsuleComponent);

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(RootComponent); // or CapsuleComponent if needed

	SwordMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordEffectMesh"));
	SwordMesh->SetupAttachment(GetMesh());
	SwordMesh->SetVisibility(false);
	SwordMesh->SetupAttachment(GetMesh(), TEXT("WeaponLeftHandSocket"));

	PlayerFollowSplineComponent = CreateDefaultSubobject<UPlayerFollowSplineComponent>(TEXT("PlayerFollowSplineComponent"));

	HeatSystem = CreateDefaultSubobject<UHeatSystemComponent>(TEXT("HeatSystem"));
	HeatSystem->OnHeatLevelChanged.AddDynamic(this, &AVanquishCharacter::HandleHeatLevelChanged);
}

// Called when the game starts or when spawned
void AVanquishCharacter::BeginPlay()
{
	Super::BeginPlay();
	UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(GetMesh()->GetOverlayMaterial(), nullptr);
	GetMesh()->SetOverlayMaterial(MaterialInstance);
	MaterialInstance->SetScalarParameterValue(FName{ "HitEffectStrength" }, 0.0);

	TArray<AActor*> attached_actors;
	GetAttachedActors(attached_actors);

	for (AActor* actor : attached_actors) {
		if (AWeaponBase* weapon = Cast<AWeaponBase>(actor)) {
			weapon->SetHeatSystemComponent(HeatSystem);
			Weapons.Add(weapon);
		}
	}
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

void AVanquishCharacter::TriggerSwordAttack()
{
	if (!HasAbilityFlag(ECharacterAbilityFlags::CanMeleeAttack)) {
		return;
	}

	RemoveAbilityFlags({ ECharacterAbilityFlags::CanMeleeAttack, ECharacterAbilityFlags::CanShoot });
	StartSwordAttack(SwordAttackNone);
	if (SwordSlashProjectileHorizontalClass)
	{
		FVector start = GetActorLocation() + GetActorForwardVector() * 100.f;
		FVector dir = GetActorForwardVector();

		FActorSpawnParameters params;
		params.Owner = this;

		FTransform spawn_transform;
		spawn_transform.SetLocation(start);
		spawn_transform.SetRotation(dir.ToOrientationQuat()); // optional, for slash alignment

		ASwordSlashProjectile* slash = GetWorld()->SpawnActor<ASwordSlashProjectile>(
			SwordSlashProjectileHorizontalClass,
			spawn_transform,
			params
			);

		if (slash)
		{
			slash->InitializeRandomArcSlash(start, dir, 0.25f, 2000.f);
			slash->OnSlashDestroyed.AddDynamic(this, &AVanquishCharacter::SwordSlashEnded);
		}
	}

	GetWorldTimerManager().ClearTimer(combo_reset_timer_handle);
	GetWorldTimerManager().SetTimer(combo_reset_timer_handle, this, &AVanquishCharacter::OnComboReset, combo_max_delay);
}

void AVanquishCharacter::OnComboReset()
{
	if (b_attack_buffered)
	{
		return; // Don't reset combo if input is buffered
	}

	e_current_sword_attack = SwordAttackNone;
	b_attack_buffered = false;
	b_is_attacking = false;
}

void AVanquishCharacter::StartSwordAttack(SwordAttackType const attack_to_start)
{
	b_is_attacking = true;
	e_current_sword_attack = attack_to_start;
	if (SwordMesh) {
		SwordMesh->SetVisibility(true);
	}
}

void AVanquishCharacter::EndSwordAttack()
{
	b_is_attacking = false;
	if (SwordMesh) {
		SwordMesh->SetVisibility(false);
	}

	// If input was buffered and not at final attack, trigger next
	if (b_attack_buffered && e_current_sword_attack != SwordAttack2)
	{
		TriggerSwordAttack();
	}
	else
	{
		// End combo
		e_current_sword_attack = SwordAttackNone;
		b_attack_buffered = false;
	}
}

void AVanquishCharacter::SwordSlashEnded() {
	AddAbilityFlags({ ECharacterAbilityFlags::CanMeleeAttack, ECharacterAbilityFlags::CanShoot });
	SetSwordSlashCurrentPos(FVector{ 0,0,0 });
	EndSwordAttack();
}


float AVanquishCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) {
	if (GetDodgingStatus())
	{
		if (!b_has_triggered_dodge_slowmo)
		{
			b_has_triggered_dodge_slowmo = true;
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.1f);
			TriggerDodgeSlowMoVFX(true);
			GetWorldTimerManager().ClearTimer(timer_handle_reset_slowmo);
			GetWorldTimerManager().SetTimer(timer_handle_reset_slowmo, this, &AVanquishCharacter::ResetGlobalTimeDilation, 0.05f, false);

			TArray<AActor*> owned_actors;
			GetAttachedActors(owned_actors);
			for (AActor* actor : owned_actors)
			{
				AWeaponBase* weapon = Cast<AWeaponBase>(actor);
				if (weapon)
				{
					weapon->ResetChargedShot();
				}
			}
		}

		return 0.f;
	}

	ToggleGlow(true);
	FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &AVanquishCharacter::ToggleGlow, false);
	GetWorldTimerManager().SetTimer(TimerHandle_TimeForHitGlow, RespawnDelegate, 1.f, false, 1.f);
	PlayAnimMontage(HitAnimMontage);
	DeactivateHitboxesFor(1.f);

	mCurrentHealth -= DamageAmount;
	if (mCurrentHealth <= 0.0f)
	{
		Die();
	}

	return DamageAmount;
}

void AVanquishCharacter::ToggleGlow(bool const activation) {
	auto const overlay_instance = Cast<UMaterialInstanceDynamic>(GetMesh()->GetOverlayMaterial());
	overlay_instance->SetScalarParameterValue(FName{ "HitEffectStrength" }, activation ? 1.0f : 0.0f);

	//GetWorldTimerManager().ClearTimer(TimerHandle_TimeForHitGlow);
}


void AVanquishCharacter::Die() {
	// Disable collision
	//GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Disable movement
	//auto CharMovement = GetCharacterMovement();
	//if (CharMovement != nullptr)
	//{
	//	CharMovement->StopMovementImmediately();
	//}

	// Play death animation and destroy actor after a delay
	//GetMesh()->PlayAnimation(DeathAnimation, false);
	//
	//FTimerHandle TimerHandle;
	//GetWorldTimerManager().SetTimer(TimerHandle, this, &AEnemyBase::DestroyEnemy, DestroyDelay);
	//GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

float AVanquishCharacter::PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	UAnimInstance* AnimInstance = (GetMesh()) ? GetMesh()->GetAnimInstance() : nullptr;
	if (AnimMontage && AnimInstance)
	{
		float const Duration = AnimInstance->Montage_Play(AnimMontage, InPlayRate);

		if (Duration > 0.f)
		{
			// Start at a given Section.
			if (StartSectionName != NAME_None)
			{
				AnimInstance->Montage_JumpToSection(StartSectionName, AnimMontage);
			}

			return Duration;
		}
	}

	return 0.f;
}

void AVanquishCharacter::StopAnimMontage(class UAnimMontage* AnimMontage)
{
	UAnimInstance* AnimInstance = (GetMesh()) ? GetMesh()->GetAnimInstance() : nullptr;
	UAnimMontage* MontageToStop = (AnimMontage) ? AnimMontage : GetCurrentMontage();
	bool bShouldStopMontage = AnimInstance && MontageToStop && !AnimInstance->Montage_GetIsStopped(MontageToStop);

	if (bShouldStopMontage)
	{
		AnimInstance->Montage_Stop(MontageToStop->BlendOut.GetBlendTime(), MontageToStop);
	}
}

class UAnimMontage* AVanquishCharacter::GetCurrentMontage()
{
	UAnimInstance* AnimInstance = (GetMesh()) ? GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInstance)
	{
		return AnimInstance->GetCurrentActiveMontage();
	}

	return nullptr;
}

void AVanquishCharacter::SetDodgingStatus(bool const new_status) {
	b_is_dodging = new_status;
	b_has_triggered_dodge_slowmo = false;
}


void AVanquishCharacter::DeactivateHitboxesFor(float const seconds) {
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Reset per-dodge slowmo trigger

	GetWorldTimerManager().ClearTimer(collision_restore_timer_handle);
	GetWorldTimerManager().SetTimer(collision_restore_timer_handle, this, &AVanquishCharacter::ReactivatePlayerHitboxes, seconds, false);
}

void AVanquishCharacter::ReactivatePlayerHitboxes() {
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndProbe);
}

void AVanquishCharacter::ResetGlobalTimeDilation() {
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
	TriggerDodgeSlowMoVFX(false);
}

void AVanquishCharacter::HandleHeatLevelChanged(int32 new_heat_value, bool is_increase) {
	auto effect_to_play = is_increase ? HeatUpVFX : HeatDownVFX;
	PlayBoneEffect(effect_to_play, "Spine", true);
	PlayBoneEffect(effect_to_play, "LeftLeg", true);
	PlayBoneEffect(effect_to_play, "RightLeg", true);
	PlayBoneEffect(effect_to_play, "LeftArm", true);
	PlayBoneEffect(effect_to_play, "RightArm", true);
}

void AVanquishCharacter::PlayBoneEffect(UNiagaraSystem* effect, FName bone_name, bool force_refresh)
{
	if (!effect || !GetMesh()) return;

	if (ActiveBoneEffects.Contains(bone_name) && ActiveBoneEffects[bone_name] && force_refresh)
	{
		ActiveBoneEffects[bone_name]->Deactivate();
		ActiveBoneEffects[bone_name]->DestroyComponent();
		ActiveBoneEffects.Remove(bone_name);
	}

	if (!ActiveBoneEffects.Contains(bone_name))
	{
		UNiagaraComponent* effect_component = UNiagaraFunctionLibrary::SpawnSystemAttached(
			effect,
			GetMesh(),
			bone_name,
			FVector::ZeroVector,
			FRotator{180,0,0},
			EAttachLocation::SnapToTargetIncludingScale,
			true
		);

		if (effect_component)
		{
			effect_component->SetAutoActivate(true);
			ActiveBoneEffects.Add(bone_name, effect_component);
		}
	}
}