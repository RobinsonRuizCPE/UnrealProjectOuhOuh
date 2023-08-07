// Fill out your copyright notice in the Description page of Project Settings.


#include "BT_Task_UpdatePositionMatrix.h"
#include "GameFramework/Actor.h" 	

#include "Components/TimelineComponent.h"

#include <Engine/Private/InterpolateComponentToAction.h>
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "EnemyBase.h"


void UBT_Task_UpdatePositionMatrix::update_matrix_pos(FVector const owner_pos) {
	auto const player_controller = GetWorld()->GetFirstPlayerController();
	if (!player_controller) {
		return;
	}

	if (!player_controller->GetPawn()) {
		return;
	}

	MatrixPos = player_controller->GetPawn()->GetActorLocation() + (player_controller->GetPawn()->GetActorForwardVector() *1000);

	
	auto result = FVector2D(FMath::FRandRange(-400,400), FMath::FRandRange(-400, 400));

	MatrixPos.X += result.X;
	MatrixPos.Z += result.Y;
}

void UBT_Task_UpdatePositionMatrix::MovementFinished() {
	FinishLatentTask(*mBehaviorTreeComp, EBTNodeResult::Type::Succeeded);
}

EBTNodeResult::Type UBT_Task_UpdatePositionMatrix::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	mBehaviorTreeComp = &OwnerComp;
	auto enemy = OwnerComp.GetBlackboardComponent()->GetValueAsObject(FName("SelfActor"));
	if (!enemy) {
		return EBTNodeResult::Type::Failed;
	}

	if (auto enemy_actor = Cast<AEnemyBase>(enemy)) {
		update_matrix_pos(enemy_actor->GetActorLocation());
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(FName("matrix position"), MatrixPos);

		// Create Latent Info
		FLatentActionInfo LatentActionInfo;
		LatentActionInfo.CallbackTarget = this;
		LatentActionInfo.ExecutionFunction = FName(TEXT("MovementFinished"));
		LatentActionInfo.UUID = GetNextUUID();
		LatentActionInfo.Linkage = 0;
		move_enemy_to(enemy_actor->GetRootComponent(), MatrixPos, true, true, 2.f, false, LatentActionInfo);
	}

	return EBTNodeResult::Type::InProgress;
}

void UBT_Task_UpdatePositionMatrix::move_enemy_to(USceneComponent* Component, FVector TargetRelativeLocation, bool bEaseOut, bool bEaseIn, float OverTime, bool bForceShortestRotationPath, FLatentActionInfo LatentInfo) {
	if (UWorld* World = GEngine->GetWorldFromContextObject(Component, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FInterpolateComponentToAction* Action = LatentActionManager.FindExistingAction<FInterpolateComponentToAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		const FVector ComponentLocation = (Component != NULL) ? Component->GetRelativeLocation() : FVector::ZeroVector;

		// If not currently running
		if (Action == NULL)
		{
			// Only act on a 'move' input if not running
			Action = new FInterpolateComponentToAction(OverTime, LatentInfo, Component, bEaseOut, bEaseIn, bForceShortestRotationPath);

			Action->TargetLocation = TargetRelativeLocation;
			Action->InitialLocation = ComponentLocation;
			Action->bInterpRotation = false;

			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}


