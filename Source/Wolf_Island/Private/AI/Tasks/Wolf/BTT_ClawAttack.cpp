// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/Wolf/BTT_ClawAttack.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

EBTNodeResult::Type UBTT_ClawAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AICon = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());

	AIPawn = AICon.IsValid() ? Cast<ACharacter>(AICon->GetPawn()) : nullptr;

	// [Refactor] Tick 안전성: 비동기 콜백 전 포인터 유효성 체크
	if (!AICon.IsValid()) return EBTNodeResult::Failed;

	if (!AIPawn.IsValid()) return EBTNodeResult::Failed;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(AIPawn.Get(), 0);

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
	// [Refactor] 비동기 콜백 진입부: OwnerComp 유효성 체크
	if (Montage == ClawAttackMontage && OwnerComp)
	{
		FinishLatentTask(*OwnerComp, bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
	}
}

