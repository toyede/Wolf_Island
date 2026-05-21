// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/Wolf/BTT_SetStateAsPassive.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

EBTNodeResult::Type UBTT_SetStateAsPassive::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AICon = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());

	AIPawn = AICon.IsValid() ? Cast<ACharacter>(AICon->GetPawn()) : nullptr;

	// [Refactor] 외부 참조 포인터 유효성 체크
	if (!AICon.IsValid()) return EBTNodeResult::Failed;

	if (!AIPawn.IsValid()) return EBTNodeResult::Failed;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(AIPawn.Get(), 0);

	AICon->SetEnemyState(EEnemyState::Passive);

	return EBTNodeResult::Succeeded;
}
