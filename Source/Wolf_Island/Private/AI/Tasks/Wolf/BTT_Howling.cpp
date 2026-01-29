// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/Wolf/BTT_Howling.h"
#include "AI/Enemy_Character/EnemyAIBase.h"
#include "AI/Interfaces/EnemyCommonInterface.h"

UBTT_Howling::UBTT_Howling()
{
	NodeName = TEXT("Howling");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTT_Howling::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		return EBTNodeResult::Failed;
	}

	AEnemyAIBase* Enemy = AICon ? Cast<AEnemyAIBase>(AICon->GetPawn()) : nullptr;

	CachedEnemy = Enemy;
	CachedOwnerComp = &OwnerComp;

	Enemy->OnHowlingEnd.AddDynamic(this, &UBTT_Howling::OnHowlingFinished);

	if (Enemy->Implements<UEnemyCommonInterface>())
	{
		IEnemyCommonInterface::Execute_Howling(Enemy);
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTT_Howling::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (CachedEnemy.IsValid())
	{
		CachedEnemy->OnHowlingEnd.RemoveDynamic(this, &UBTT_Howling::OnHowlingFinished);
	}

	return EBTNodeResult::Aborted;
}

void UBTT_Howling::OnHowlingFinished()
{
	if (CachedEnemy.IsValid())
	{
		CachedEnemy->OnHowlingEnd.RemoveDynamic(this, &UBTT_Howling::OnHowlingFinished);
	}
	if (CachedOwnerComp.IsValid())
	{
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
	}
}
