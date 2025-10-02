// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/Wolf/BTT_ClawAttack.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

EBTNodeResult::Type UBTT_ClawAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	ACharacter* AIPawn = AICon ? Cast<ACharacter>(AICon->GetPawn()) : nullptr;

	if (!AIPawn) return EBTNodeResult::Failed;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(AIPawn, 0);

	if (!PlayerPawn) return EBTNodeResult::Failed;

	UAnimInstance* AnimInstance = AIPawn->GetMesh()->GetAnimInstance();

	return EBTNodeResult::Type();
}
