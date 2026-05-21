// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/Boss/BTT_ThrowSkeleton.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Actors/StoneProjectile.h"

EBTNodeResult::Type UBTT_ThrowSkeleton::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AICon = Cast<AEnemyAIBossController>(OwnerComp.GetAIOwner());

	AIPawn = AICon.IsValid() ? Cast<ACharacter>(AICon->GetPawn()) : nullptr;

	// [Refactor] 외부 참조 포인터 유효성 체크
	if (!AIPawn.IsValid()) return EBTNodeResult::Failed;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(AIPawn.Get(), 0);

	if (!PlayerPawn) return EBTNodeResult::Failed;

	UAnimInstance* AnimInstance = AIPawn->GetMesh()->GetAnimInstance();

	if (AnimInstance && ThrowSkeletonMontage)
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UBTT_ThrowSkeleton::OnMontageEnded, &OwnerComp);

		AnimInstance->Montage_Play(ThrowSkeletonMontage);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, ThrowSkeletonMontage);

		if (ThrowSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ThrowSound, PlayerPawn->GetActorLocation());
		}
		return EBTNodeResult::InProgress;
	}


	return EBTNodeResult::Failed;
}

void UBTT_ThrowSkeleton::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp)
{
	// [Refactor] 비동기 콜백 진입부: OwnerComp 유효성 체크
	if (Montage == ThrowSkeletonMontage && OwnerComp && !bInterrupted)
	{
		FinishLatentTask(*OwnerComp, bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
	}
}