// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/Wolf/BTT_ThrowStone.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

EBTNodeResult::Type UBTT_ThrowStone::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	ACharacter* AIPawn = AICon ? Cast<ACharacter>(AICon->GetPawn()) : nullptr;

	if (!AIPawn) return EBTNodeResult::Failed;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(AIPawn, 0);

	if (!PlayerPawn) return EBTNodeResult::Failed;

	UAnimInstance* AnimInstance = AIPawn->GetMesh()->GetAnimInstance();

	if (AnimInstance && ThrowStoneMontage)
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UBTT_ThrowStone::OnMontageEnded, &OwnerComp);
		
		AnimInstance->Montage_Play(ThrowStoneMontage);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, ThrowStoneMontage);

		return EBTNodeResult::InProgress;
	}


	return EBTNodeResult::Failed;
}

void UBTT_ThrowStone::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp)
{
	if (Montage == ThrowStoneMontage && OwnerComp)
	{
		FinishLatentTask(*OwnerComp, bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
	}
}