// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SwordAttack/SwordSlashProjectile.h"
#include "VanquishCharacter.h"

#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

ASwordSlashProjectile::ASwordSlashProjectile()
{
    PrimaryActorTick.bCanEverTick = true;

    HitBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("SlashCapsule"));
    SetRootComponent(HitBoxComp);
    HitBoxComp->InitBoxExtent(FVector(1.f, 1.f, 1.f));
    ProjectileMovementComponent->bAutoActivate = false;

    SetProjectileMaxDistance(20000);
}

void ASwordSlashProjectile::InitializeLaserFromTo(USceneComponent * target_origin_component, USceneComponent * target_start_component, USceneComponent* target_end_component, FName socket_name, FVector direction, float duration, float override_additional_box_exent = 0.f, bool looping)
{
    SlashAudio = UGameplayStatics::SpawnSoundAttached(SlashSound, GetRootComponent(), NAME_None, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true, 1.0f, 1.f, 0.0f, SoundAttenuation);

    life_duration = duration;
    life_timer = 0.f;
    LaserOriginComp = target_origin_component;
    LaserOriginCompSocketName = socket_name;
    LaserStartComp = target_start_component;
    LaserEndComp = target_end_component;

    ShouldUpdateBoxExtent = true;
    if (!FMath::IsNearlyZero(override_additional_box_exent)) {
        AddtionalBoxExtent = override_additional_box_exent;
    }
    eSlashType = eSlashMethod::Laser;
    Looping = looping;
}

void ASwordSlashProjectile::InitializeRandomArcSlash(FVector origin, FVector direction, float duration, float speed)
{
    SetActorLocation(origin);
    SetActorRotation(direction.Rotation());
    SlashAudio = UGameplayStatics::SpawnSoundAttached(SlashSound, GetRootComponent(), NAME_None, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true, 1.0f, 1.f, 0.0f, SoundAttenuation);

    initial_location = origin;
    FRotator forward_rot = direction.Rotation();
    auto const title_angle = FMath::FRandRange(-180.f,180.f);
    slash_start_rot = forward_rot + FRotator(title_angle, -90, 0); // start from left
    slash_end_rot = forward_rot + FRotator(-title_angle, 90, 0);    // end on right

    life_duration = duration;
    life_timer = 0.f;
    eSlashType = eSlashMethod::RandomArc;
    HitBoxComp->InitBoxExtent(FVector(1000.f, 100.f, 20.f));
    HitBoxComp->AddLocalOffset(FVector(-1000.f, 0.f, 0.f));
    HitBoxComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    HitBoxComp->SetUsingAbsoluteRotation(true);
    HitBoxComp->SetUsingAbsoluteLocation(true);
}

void ASwordSlashProjectile::InitializeLaserConstantSpeed(USceneComponent* target_origin_component, USceneComponent* target_start_component, USceneComponent* target_end_component, FName socket_name, FVector direction, float duration, float speed, float override_additional_box_extent = 0.f)
{
    SlashAudio = UGameplayStatics::SpawnSoundAttached(SlashSound, GetRootComponent(), NAME_None, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true, 1.0f, 1.f, 0.0f, SoundAttenuation);

    life_duration = duration;
    life_timer = 0.f;
    LaserOriginComp = target_origin_component;
    LaserOriginCompSocketName = socket_name;
    LaserStartComp = target_start_component;
    LaserEndComp = target_end_component;
    laser_speed = speed;

    ShouldUpdateBoxExtent = true;
    if (!FMath::IsNearlyZero(override_additional_box_extent)) {
        AddtionalBoxExtent = override_additional_box_extent;
    }

    lerped_position_cache = LaserStartComp->GetComponentLocation();
    eSlashType = eSlashMethod::LaserConstantSpeed;
}

void ASwordSlashProjectile::BeginPlay()
{
    Super::BeginPlay();
    HitBoxComp->InitBoxExtent(FVector(1.f, 1.f, 1.f));

    HitBoxComp->OnComponentBeginOverlap.AddDynamic(this, &ASwordSlashProjectile::HandleOverlap);
    HitBoxComp->OnComponentHit.AddDynamic(this, &ASwordSlashProjectile::HandleHit);
    if (GetOwner())
    {
        CollisionSphere->IgnoreActorWhenMoving(GetOwner(), true);
    }


    if (SlashTraceEffect) {
        SlashTraceEffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            SlashTraceEffect,
            HitBoxComp,           // Attach to the mesh so it follows properly
            NAME_None,                // Attach to socket name, none in this case
            FVector::ZeroVector,      // Relative location
            FRotator::ZeroRotator,    // Relative rotation
            EAttachLocation::SnapToTarget,
            true                      // Auto destroy with parent
        );

        SlashTraceEffectComponent->SetAutoActivate(false); // Don't play until we tell it
    }

    if (SlashTraceEffect && SlashTraceEffectComponent) {
        SlashTraceEffectComponent->SetAsset(SlashTraceEffect);
        SlashTraceEffectComponent->Activate(true);
        
        SlashTraceEffectComponent->SetFloatParameter("TrailWidth", HitBoxComp->GetScaledBoxExtent().X * 2.5);
    }
}

void ASwordSlashProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (HitBoxComp)  // or CollisionShape if renamed
    {
        FVector loc = HitBoxComp->GetComponentLocation();
        FQuat rot = HitBoxComp->GetComponentQuat();
        //DrawDebugBox(GetWorld(), loc, HitBoxComp->GetUnscaledBoxExtent(), rot, FColor::Cyan, true, -1.f, 0, 2.0f);
    }

    switch (eSlashType) {
    case eSlashMethod::RandomArc:
        UpdateRandomArc();
        break;
    case eSlashMethod::Laser:
        UpdateLaser();
        break;
    case eSlashMethod::LaserConstantSpeed:
        UpdateConstantSpeed(DeltaTime);
        break;
    default:
        break;
    }

    life_timer += DeltaTime;

    if (Looping) {
        return;
    }

    if (life_timer >= life_duration)
    {
        Destroy();
    }
}


void ASwordSlashProjectile::UpdateRandomArc() {
    float alpha = FMath::Clamp(life_timer / life_duration, 0.f, 1.f);
    FQuat start_quat = slash_start_rot.Quaternion();
    FQuat end_quat = slash_end_rot.Quaternion();
    FQuat current_quat = FQuat::SlerpFullPath(start_quat, end_quat, alpha);

    FRotator current_rot = current_quat.Rotator();
    if (ShouldUpdateBoxExtent) {
        FVector lerped_pos = FMath::Lerp(slash_from, slash_to, alpha);
        float length = FVector::Distance(initial_location, lerped_pos) * 0.5f;
        HitBoxComp->SetBoxExtent(FVector(length + AddtionalBoxExtent, HitBoxComp->GetUnscaledBoxExtent().Y, HitBoxComp->GetUnscaledBoxExtent().Z));
        HitBoxComp->SetRelativeLocation(FVector::ZeroVector);
        HitBoxComp->AddLocalOffset(FVector(-length, 0.f, 0.f));
        lerped_position_cache = lerped_pos;
    }

    // Always rotate around the origin/player, with a fixed radius
    float slash_radius = HitBoxComp->GetUnscaledBoxExtent().X;
    FVector offset = current_rot.RotateVector(FVector(slash_radius, 0, 0)); // arc around player

    FVector new_location = initial_location + offset;
    SetActorLocation(new_location, true);
    SetActorRotation(current_rot); // optional
    HitBoxComp->SetBoxExtent(HitBoxComp->GetUnscaledBoxExtent(), true);

    if (auto player = Cast<AVanquishCharacter>(GetOwner())) {
        player->SetSwordSlashCurrentPos(new_location);
    }
}

void ASwordSlashProjectile::UpdateLaser() {
    if (!LaserOriginComp || !LaserStartComp || !LaserEndComp) {
        return;
    }

    float alpha = FMath::Clamp(life_timer / life_duration, 0.f, 1.f);
    if (alpha >= 1.f && Looping) {
        life_timer = 0;
        CurrentLaserLoop = !CurrentLaserLoop;
        alpha = FMath::Clamp(life_timer / life_duration, 0.f, 1.f);
    }
    auto const target_start_location = LaserStartComp->GetComponentLocation();
    auto const target_end_location = LaserEndComp->GetComponentLocation();
    FVector current_end = FMath::Lerp(CurrentLaserLoop ? target_end_location : target_start_location, CurrentLaserLoop ? target_start_location : target_end_location, alpha);
    FVector laser_origin = LaserOriginComp->GetSocketLocation(LaserOriginCompSocketName);

    FVector laser_vec = current_end - laser_origin;
    FVector laser_dir = laser_vec.GetSafeNormal();

    float laser_length = laser_vec.Size() + AddtionalBoxExtent;
    FVector box_center = laser_origin + 0.5f * laser_vec + AddtionalBoxExtent * laser_dir * 0.5;
    FRotator box_rotation = laser_dir.Rotation();
    FVector box_extent = FVector(laser_length * 0.5f , 20.f, 20.f);

    // Apply completely independent transform
    HitBoxComp->SetWorldLocation(box_center, true);
    HitBoxComp->SetWorldRotation(box_rotation, true);
    //HitBoxComp->SetRelativeRotation(FRotator{ life_timer * 5 , 0 , 0 }, true);
    HitBoxComp->SetBoxExtent(box_extent, true);

    // Optional debug
    DrawDebugBox(GetWorld(), box_center, box_extent, box_rotation.Quaternion(), FColor::Cyan, false, -1.f, 0, 2.f);

    lerped_position_cache = current_end;
    if (auto player = Cast<AVanquishCharacter>(GetOwner())) {
        player->SetSwordSlashCurrentPos(current_end);
    }
}

void ASwordSlashProjectile::UpdateConstantSpeed(float delta_time) {
    FVector laser_origin = LaserOriginComp->GetSocketLocation(LaserOriginCompSocketName);
    auto const laser_direction = (LaserEndComp->GetComponentLocation() - lerped_position_cache).GetSafeNormal();

    lerped_position_cache += laser_direction * laser_speed * delta_time;

    FVector laser_vec = lerped_position_cache - laser_origin;
    FVector laser_dir = laser_vec.GetSafeNormal();

    float laser_length = laser_vec.Size() + AddtionalBoxExtent;
    FVector box_center = laser_origin + 0.5f * laser_vec + AddtionalBoxExtent * laser_dir * 0.5;
    FRotator box_rotation = laser_dir.Rotation().Add(delta_time, 0, 0);
    FVector box_extent = FVector(laser_length * 0.5f, 20.f, 20.f);

    HitBoxComp->SetWorldLocation(box_center, true);
    HitBoxComp->SetWorldRotation(box_rotation, true);
    HitBoxComp->SetBoxExtent(box_extent, true);

    //DrawDebugBox(GetWorld(), box_center, box_extent, box_rotation.Quaternion(), FColor::Red, false, -1.f, 0, 2.f);
    if (auto player = Cast<AVanquishCharacter>(GetOwner())) {
        player->SetSwordSlashCurrentPos(lerped_position_cache);
    }
}

void ASwordSlashProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason) {
    if (SlashAudio) {
        if (SlashAudio->IsPlaying()) {
            SlashAudio->FadeOut(1, 0);
        }
    }

    if (auto player = Cast<AVanquishCharacter>(GetOwner())) {
        player->SetSwordSlashCurrentPos(FVector{0,0,0});
        player->EndSwordAttack();

    }
}


void ASwordSlashProjectile::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnOverlap(OverlappedComponent, OtherActor,
        OtherComp, OtherBodyIndex,
        bFromSweep, SweepResult);
    //HandleProjectileImpact(OtherActor, SweepResult.ImpactPoint, SweepResult);
}

void ASwordSlashProjectile::HandleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse,
    const FHitResult& Hit)
{
    HandleProjectileImpact(OtherActor, Hit.ImpactPoint, Hit);
}
