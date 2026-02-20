// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_SummonStatue.h"
#include "AIController.h"
#include "AI/Enemy_Character/EnemyAIBoss.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SummonStatue::UBTTask_SummonStatue()
{
	NodeName = TEXT("Summon Statue");
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_SummonStatue::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    Boss->OnSummonStatueEnd.AddDynamic(this, &UBTTask_SummonStatue::OnSummonStatueEnd);
	Boss->ExecuteSummonStatue();

    return EBTNodeResult::InProgress;
}

void UBTTask_SummonStatue::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    if (CachedBoss.IsValid())
    {
        CachedBoss->OnSummonStatueEnd.RemoveDynamic(this, &UBTTask_SummonStatue::OnSummonStatueEnd);
    }

    CachedOwnerComp.Reset();
    CachedBoss.Reset();
}

void UBTTask_SummonStatue::OnSummonStatueEnd()
{
    if (CachedOwnerComp.IsValid())
    {
        FinishLatentTask(*CachedOwnerComp.Get(), EBTNodeResult::Succeeded);
    }
}
