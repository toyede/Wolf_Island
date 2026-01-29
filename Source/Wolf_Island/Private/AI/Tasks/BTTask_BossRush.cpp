// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_BossRush.h"
#include "AIController.h"
#include "AI/Enemy_Character/EnemyAIBoss.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_BossRush::UBTTask_BossRush()
{
	NodeName = TEXT("Boss Rush");
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_BossRush::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	Boss->OnBossRushEnd.AddDynamic(this, &UBTTask_BossRush::OnBossRushEnd);
	Boss->ExecuteRush();

	return EBTNodeResult::InProgress;
}

void UBTTask_BossRush::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	if (CachedBoss.IsValid())
	{
		CachedBoss->OnBossRushEnd.RemoveDynamic(this, &UBTTask_BossRush::OnBossRushEnd);
	}

	CachedOwnerComp.Reset();
	CachedBoss.Reset();
}

void UBTTask_BossRush::OnBossRushEnd()
{
	if (CachedOwnerComp.IsValid())
	{
		FinishLatentTask(*CachedOwnerComp.Get(), EBTNodeResult::Succeeded);
	}
}


