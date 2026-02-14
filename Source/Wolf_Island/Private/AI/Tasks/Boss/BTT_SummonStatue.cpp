// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/Boss/BTT_SummonStatue.h"
#include "AIController.h"
#include "AI/Enemy_Character/EnemyAIBoss.h"

EBTNodeResult::Type UBTT_SummonStatue::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	APawn* AIPawn = AICon ? AICon->GetPawn() : nullptr;
	AEnemyAIBoss* Boss = AIPawn ? Cast<AEnemyAIBoss>(AIPawn) : nullptr;

	if (!Boss)
	{
		return EBTNodeResult::Failed;
	}

	Boss->ExecuteSummonStatue();
	return EBTNodeResult::Succeeded;
}
