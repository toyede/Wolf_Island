// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_SpecialAttack.h"
#include "AIController.h"
#include "AI/Enemy_Character/EnemyAIBoss.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SpecialAttack::UBTTask_SpecialAttack()
{
	NodeName = TEXT("Special Attack");
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_SpecialAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	Boss->OnSpecialAttackEnd.AddDynamic(this, &UBTTask_SpecialAttack::OnSpecialAttackEnd);
	Boss->ExecuteSpecialAttack();
	return EBTNodeResult::InProgress;
}

void UBTTask_SpecialAttack::OnSpecialAttackEnd()
{
	if (CachedOwnerComp.IsValid())
	{
		FinishLatentTask(*CachedOwnerComp.Get(), EBTNodeResult::Succeeded);
	}
}

void UBTTask_SpecialAttack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	if (CachedBoss.IsValid())
	{
		CachedBoss->OnSpecialAttackEnd.RemoveDynamic(this, &UBTTask_SpecialAttack::OnSpecialAttackEnd);
	}

	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		BB->ClearValue(TEXT("AttackTarget"));
	}

	CachedOwnerComp.Reset();
	CachedBoss.Reset();
}