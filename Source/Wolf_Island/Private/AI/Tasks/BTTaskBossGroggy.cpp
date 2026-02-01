// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTaskBossGroggy.h"
#include "AIController.h"
#include "AI/Enemy_Character/EnemyAIBoss.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTaskBossGroggy::UBTTaskBossGroggy()
{
	NodeName = TEXT("Boss Groggy");
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTaskBossGroggy::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	Boss->OnBossGroggyEnd.AddDynamic(this, &UBTTaskBossGroggy::OnBossGroggyEnd);
    Boss->ExecuteGroggy();

	return EBTNodeResult::InProgress;
}

void UBTTaskBossGroggy::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    if (CachedBoss.IsValid())
    {
        CachedBoss->OnBossGroggyEnd.RemoveDynamic(this, &UBTTaskBossGroggy::OnBossGroggyEnd);
    }

    if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
    {
        BB->ClearValue(TEXT("AttackTarget"));
    }
    CachedOwnerComp.Reset();
    CachedBoss.Reset();
}

void UBTTaskBossGroggy::OnBossGroggyEnd()
{
    if (CachedOwnerComp.IsValid())
    {
        FinishLatentTask(*CachedOwnerComp.Get(), EBTNodeResult::Succeeded);
    }
}
