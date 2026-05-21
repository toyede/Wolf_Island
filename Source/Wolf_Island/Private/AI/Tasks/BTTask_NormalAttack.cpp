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

    AEnemyAIBase* Enemy = Cast<AEnemyAIBase>(AICon->GetPawn());

    // [Refactor] 비동기 콜백 준비 전: Enemy 유효성 체크
    if (!IsValid(Enemy))
    {
        UE_LOG(LogTemp, Warning, TEXT("[Refactor] UBTTask_NormalAttack::ExecuteTask: Enemy is invalid"));
        return EBTNodeResult::Failed;
    }

    CachedEnemy = Enemy;
    CachedOwnerComp = &OwnerComp;

    Enemy->OnAttackEnd.AddDynamic(this, &UBTTask_NormalAttack::OnAttackFinished);

    if (Enemy->Implements<UEnemyCommonInterface>())
    {
        IEnemyCommonInterface::Execute_NormalAttack(Enemy);
        return EBTNodeResult::InProgress;
    }

    // [Refactor] 인터페이스 미구현 시 AddDynamic 정리 후 실패 반환
    Enemy->OnAttackEnd.RemoveDynamic(this, &UBTTask_NormalAttack::OnAttackFinished);
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

