// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_SummonWolves.h"
#include "AIController.h"
#include "AI/Enemy_Character/EnemyAIBoss.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_SummonWolves::UBTTask_SummonWolves()
{
	NodeName = TEXT("Summon Wolves");
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_SummonWolves::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		return EBTNodeResult::Failed;
	}

	AEnemyAIBoss* Boss = Cast<AEnemyAIBoss>(AICon->GetPawn());
	if (!Boss)
	{
		return EBTNodeResult::Failed;
	}

	CachedOwnerComp = &OwnerComp;
	CachedBoss = Boss;

	Boss->OnSummonWolvesEnd.AddDynamic(this, &UBTTask_SummonWolves::OnSummonWolvesEnd);
	Boss->ExecuteSummonWolves();

	return EBTNodeResult::InProgress;
}

void UBTTask_SummonWolves::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	if (CachedBoss.IsValid())
	{
		CachedBoss->OnSummonWolvesEnd.RemoveDynamic(this, &UBTTask_SummonWolves::OnSummonWolvesEnd);
	}

	CachedOwnerComp.Reset();
	CachedBoss.Reset();
}

void UBTTask_SummonWolves::OnSummonWolvesEnd()
{
	if (CachedOwnerComp.IsValid())
	{
		FinishLatentTask(*CachedOwnerComp.Get(), EBTNodeResult::Succeeded);
	}
}

