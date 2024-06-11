// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BT_Task_UpdatePositionMatrix.generated.h"


class APawn;

UCLASS()
class MYPROJECT_API UBT_Task_UpdatePositionMatrix : public UBTTaskNode
{
	GENERATED_BODY()

private:

	FVector MatrixPos;
	FRotator MatrixRot;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	void update_matrix_pos(FVector const owner_pos);

	void move_enemy_to(USceneComponent* Component, FVector TargetRelativeLocation, bool bEaseOut, bool bEaseIn, float OverTime, bool bForceShortestRotationPath, FLatentActionInfo LatentInfo);

	UFUNCTION()
	void MovementFinished();

	int32 GetNextUUID() {
		return NextUUID++;
	}

private:
	int32 NextUUID = 0;
	UBehaviorTreeComponent* mBehaviorTreeComp;
};
