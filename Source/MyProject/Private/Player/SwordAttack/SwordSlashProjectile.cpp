// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SwordAttack/SwordSlashProjectile.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"


ASwordSlashProjectile::ASwordSlashProjectile()
{
    PrimaryActorTick.bCanEverTick = true;

    CapsuleComp = CreateDefaultSubobject<UBoxComponent>(TEXT("SlashCapsule"));
    CapsuleComp->SetupAttachment(CollisionSphere);
    //CapsuleComp->SetRelativeRotation(FRotator(0, 90, 0)); // Or whatever axis you need

    // Replace default sphere with a capsule for longer slash shape
    BoxComp->InitBoxExtent(FVector(100.f, 50.f, 10.f));
    // length, width, height    CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CapsuleComp->SetCollisionObjectType(ECC_WorldDynamic);
    CapsuleComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    CapsuleComp->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnOverlap);
    CapsuleComp->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);

    //CollisionSphere = CapsuleComp;

    // Optional: disable default movement logic
    ProjectileMovementComponent->bAutoActivate = false;

    SetProjectileMaxDistance(20000);
}

void ASwordSlashProjectile::InitializeSlash(FVector origin, FVector direction, float duration, float speed)
{
    SetActorLocation(origin);
    SetActorRotation(direction.Rotation());

    initial_location = origin;
    FRotator forward_rot = direction.Rotation();
    slash_start_rot = forward_rot + FRotator(0, -100, 0); // start from left
    slash_end_rot = forward_rot + FRotator(0, 90, 0);    // end on right

    life_duration = duration;
    life_timer = 0.f;
}

void ASwordSlashProjectile::BeginPlay()
{
    Super::BeginPlay();
    if (GetOwner())
    {
        CollisionSphere->IgnoreActorWhenMoving(GetOwner(), true);
    }

    // Optional: activate VFX if any
    if (TraceEffect && TraceEffectComponent)
    {
        TraceEffectComponent->Activate(true);
    }
}

void ASwordSlashProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    float alpha = FMath::Clamp(life_timer / life_duration, 0.f, 1.f);
    FQuat start_quat = slash_start_rot.Quaternion();
    FQuat end_quat = slash_end_rot.Quaternion();
    FQuat current_quat = FQuat::SlerpFullPath(start_quat, end_quat, alpha);

    FRotator current_rot = current_quat.Rotator();

    // Always rotate around the origin/player, with a fixed radius
    float slash_radius = CapsuleComp->GetUnscaledCapsuleHalfHeight()*2;
    FVector offset = current_rot.RotateVector(FVector(slash_radius, 0, 0)); // arc around player

    FVector new_location = initial_location + offset;
    SetActorLocation(new_location);
    SetActorRotation(current_rot); // optional

    life_timer += DeltaTime;



    if (UCapsuleComponent* capsule = CapsuleComp)  // or CollisionShape if renamed
    {
        FVector loc = capsule->GetComponentLocation();
        FQuat rot = capsule->GetComponentQuat();
        float half_height = capsule->GetUnscaledCapsuleHalfHeight();
        float radius = capsule->GetUnscaledCapsuleRadius();

        DrawDebugCapsule(GetWorld(), loc, half_height, radius, rot, FColor::Cyan, true, -1.f, 0, 2.0f);
    }

    if (life_timer >= life_duration)
    {
        Destroy();
    }
}

void ASwordSlashProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason) {

}

void ASwordSlashProjectile::MoveSlash(float DeltaTime)
{
    // Rotate the direction vector around the curve axis
    float angle_delta = curve_angle_per_sec * DeltaTime;
    slash_direction = slash_direction.RotateAngleAxis(angle_delta, curve_axis);

    FVector new_location = GetActorLocation() + (slash_direction * slash_speed * DeltaTime);
    SetActorLocation(new_location);

    if (UCapsuleComponent* capsule = CapsuleComp)  // or CollisionShape if renamed
    {
        FVector loc = capsule->GetComponentLocation();
        FQuat rot = capsule->GetComponentQuat();
        float half_height = capsule->GetUnscaledCapsuleHalfHeight();
        float radius = capsule->GetUnscaledCapsuleRadius();

        DrawDebugCapsule(GetWorld(), loc, half_height, radius, rot, FColor::Cyan, true, -1.f, 0, 2.0f);
    }

}
