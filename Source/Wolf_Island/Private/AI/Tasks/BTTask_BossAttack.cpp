// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_BossAttack.h"
#include "AIController.h"
#include "AI/Enemy_Character/EnemyAIBoss.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

UBTTask_BossAttack::UBTTask_BossAttack()
{
    NodeName = TEXT("Boss Attack");
    bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_BossAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    Boss->OnBossAttackEnd.AddDynamic(this, &UBTTask_BossAttack::OnBossAttackEnd);
    Boss->ExecuteAttack(AttackIndex);

    return EBTNodeResult::InProgress;
}

void UBTTask_BossAttack::OnBossAttackEnd()
{
    if (CachedOwnerComp.IsValid())
    {
        FinishLatentTask(*CachedOwnerComp.Get(), EBTNodeResult::Succeeded);
    }
}

void UBTTask_BossAttack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    if (CachedBoss.IsValid())
    {
        CachedBoss->OnBossAttackEnd.RemoveDynamic(this, &UBTTask_BossAttack::OnBossAttackEnd);
    }

    CachedOwnerComp.Reset();
    CachedBoss.Reset();
}

