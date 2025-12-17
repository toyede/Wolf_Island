// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/Wolf/BTT_SetStateAsPassive.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

EBTNodeResult::Type UBTT_SetStateAsPassive::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AICon = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());

	AIPawn = AICon ? Cast<ACharacter>(AICon->GetPawn()) : nullptr;

	if (!AICon) return EBTNodeResult::Failed;

	if (!AIPawn) return EBTNodeResult::Failed;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(AIPawn, 0);

	AICon->SetEnemyState(EEnemyState::Passive);

	return EBTNodeResult::Succeeded;
}
