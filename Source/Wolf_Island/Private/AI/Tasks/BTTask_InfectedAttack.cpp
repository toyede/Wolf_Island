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
	AWerewolfInfected* Infected = AICon ? Cast<AWerewolfInfected>(AICon->GetPawn()) : nullptr;
	CachedInfected = Infected;
	CachedOwnerComp = &OwnerComp;
	Infected->OnAttackEnd.AddDynamic(this, &UBTTask_InfectedAttack::OnAttackFinished);
	if (Infected->Implements<UEnemyCommonInterface>())
	{
		IEnemyCommonInterface::Execute_NormalAttack(Infected);
		return EBTNodeResult::InProgress;
	}
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