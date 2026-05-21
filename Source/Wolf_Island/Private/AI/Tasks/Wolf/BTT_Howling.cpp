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

	AEnemyAIBase* Enemy = Cast<AEnemyAIBase>(AICon->GetPawn());

	// [Refactor] 비동기 콜백 준비 전: Enemy 유효성 체크
	if (!IsValid(Enemy))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Refactor] UBTT_Howling::ExecuteTask: Enemy is invalid"));
		return EBTNodeResult::Failed;
	}

	CachedEnemy = Enemy;
	CachedOwnerComp = &OwnerComp;

	Enemy->OnHowlingEnd.AddDynamic(this, &UBTT_Howling::OnHowlingFinished);

	if (Enemy->Implements<UEnemyCommonInterface>())
	{
		IEnemyCommonInterface::Execute_Howling(Enemy);
		return EBTNodeResult::InProgress;
	}

	// [Refactor] 인터페이스 미구현 시 AddDynamic 정리 후 실패 반환
	Enemy->OnHowlingEnd.RemoveDynamic(this, &UBTT_Howling::OnHowlingFinished);
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
