// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AI/EnemyAIController.h"
#include "BTT_SetStateAsPassive.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UBTT_SetStateAsPassive : public UBTTaskNode
{
	GENERATED_BODY()

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	AEnemyAIController* AICon;

	ACharacter* AIPawn;
	
};
