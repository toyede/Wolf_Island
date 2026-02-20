// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_NormalAttack.h"
#include "AI/Enemy_Character/EnemyAIBase.h"
#include "AI/Interfaces/EnemyCommonInterface.h"

UBTTask_NormalAttack::UBTTask_NormalAttack()
{
	NodeName = TEXT("Normal Attack");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_NormalAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon)
    {
        return EBTNodeResult::Failed;
    }

    AEnemyAIBase* Enemy = AICon ? Cast<AEnemyAIBase>(AICon->GetPawn()) : nullptr;

    CachedEnemy = Enemy;
    CachedOwnerComp = &OwnerComp;

    Enemy->OnAttackEnd.AddDynamic(this, &UBTTask_NormalAttack::OnAttackFinished);

    if (Enemy->Implements<UEnemyCommonInterface>())
    {
        IEnemyCommonInterface::Execute_NormalAttack(Enemy);
        return EBTNodeResult::InProgress;
    }

    return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_NormalAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 델리게이트 정리
    if (CachedEnemy.IsValid())
    {
        CachedEnemy->OnAttackEnd.RemoveDynamic(this, &UBTTask_NormalAttack::OnAttackFinished);
    }

    return EBTNodeResult::Aborted;
}

void UBTTask_NormalAttack::OnAttackFinished()
{
    if (CachedEnemy.IsValid())
    {
        CachedEnemy->OnAttackEnd.RemoveDynamic(this, &UBTTask_NormalAttack::OnAttackFinished);
    }

    if (CachedOwnerComp.IsValid())
    {
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
    }
}

