// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_Thrust.h"
#include "AIController.h"
#include "AI/Enemy_Character/EnemyAIBoss.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Thrust::UBTTask_Thrust()
{
	NodeName = TEXT("Thrust Attack");
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_Thrust::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	Boss->OnThrustEnd.AddDynamic(this, &UBTTask_Thrust::OnThrustEnd);
	Boss->ExecuteThrust();

	return EBTNodeResult::InProgress;
}

void UBTTask_Thrust::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	if (CachedBoss.IsValid())
	{
		CachedBoss->OnThrustEnd.RemoveDynamic(this, &UBTTask_Thrust::OnThrustEnd);
	}
	CachedOwnerComp.Reset();
	CachedBoss.Reset();
}

void UBTTask_Thrust::OnThrustEnd()
{
	if (CachedOwnerComp.IsValid())
	{
		FinishLatentTask(*CachedOwnerComp.Get(), EBTNodeResult::Succeeded);
	}
}