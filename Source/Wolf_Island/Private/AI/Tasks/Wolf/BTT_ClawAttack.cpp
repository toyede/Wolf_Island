// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/Wolf/BTT_ClawAttack.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

EBTNodeResult::Type UBTT_ClawAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AICon = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());

	AIPawn = AICon ? Cast<ACharacter>(AICon->GetPawn()) : nullptr;

	if (!AICon) return EBTNodeResult::Failed;

	if (!AIPawn) return EBTNodeResult::Failed;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(AIPawn, 0);

	if (!PlayerPawn) return EBTNodeResult::Failed;

	UAnimInstance* AnimInstance = AIPawn->GetMesh()->GetAnimInstance();

	if (!AnimInstance) return EBTNodeResult::Failed;

	if (AnimInstance && ClawAttackMontage)
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UBTT_ClawAttack::OnMontageEnded, &OwnerComp);

		AnimInstance->Montage_Play(ClawAttackMontage);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, ClawAttackMontage);

		return EBTNodeResult::InProgress;
	}


	return EBTNodeResult::Failed;
}

void UBTT_ClawAttack::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp)
{
	if (Montage == ClawAttackMontage && OwnerComp)
	{
		FinishLatentTask(*OwnerComp, bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
	}
}

