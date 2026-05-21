// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_InfectedAttack.h"
#include "Character/WerewolfInfected.h"
#include "AI/Interfaces/EnemyCommonInterface.h"
#include "AIController.h"

UBTTask_InfectedAttack::UBTTask_InfectedAttack()
{
	NodeName = TEXT("Infected Attack");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_InfectedAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		return EBTNodeResult::Failed;
	}

	AWerewolfInfected* Infected = Cast<AWerewolfInfected>(AICon->GetPawn());

	// [Refactor] 비동기 콜백 준비 전: Infected 유효성 체크
	if (!IsValid(Infected))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Refactor] UBTTask_InfectedAttack::ExecuteTask: Infected is invalid"));
		return EBTNodeResult::Failed;
	}

	CachedInfected = Infected;
	CachedOwnerComp = &OwnerComp;
	Infected->OnAttackEnd.AddDynamic(this, &UBTTask_InfectedAttack::OnAttackFinished);
	if (Infected->Implements<UEnemyCommonInterface>())
	{
		IEnemyCommonInterface::Execute_NormalAttack(Infected);
		return EBTNodeResult::InProgress;
	}

	// [Refactor] 인터페이스 미구현 시 AddDynamic 정리 후 실패 반환
	Infected->OnAttackEnd.RemoveDynamic(this, &UBTTask_InfectedAttack::OnAttackFinished);
	return EBTNodeResult::Failed;
}
void UBTTask_InfectedAttack::OnAttackFinished()
{
	if (CachedInfected.IsValid())
	{
		CachedInfected->OnAttackEnd.RemoveDynamic(this, &UBTTask_InfectedAttack::OnAttackFinished);
	}

	if (CachedOwnerComp.IsValid())
	{
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
	}
}