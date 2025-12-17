// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_Die.h"
#include "AIController.h"
#include "AI/Enemy_Character/EnemyAIBase.h"
#include "AI/Interfaces/EnemyCommonInterface.h" 

UBTTask_Die::UBTTask_Die()
{
	NodeName = TEXT("Die");

	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_Die::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	AEnemyAIBase* Enemy = AICon ? Cast<AEnemyAIBase>(AICon->GetPawn()) : nullptr;

	if (!Enemy) return EBTNodeResult::Failed;

	if (Enemy->Implements<UEnemyCommonInterface>())
	{
		IEnemyCommonInterface::Execute_Die(Enemy);
	}

	return EBTNodeResult::Succeeded;
}
