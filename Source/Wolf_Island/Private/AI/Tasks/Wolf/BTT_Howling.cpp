// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/Wolf/BTT_Howling.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Perception/AISense_Hearing.h"
#include "AI/Enemy_Character/EnemyAIBase.h"

EBTNodeResult::Type UBTT_Howling::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AICon = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());

	AIPawn = AICon ? Cast<AEnemyAIBase>(AICon->GetPawn()) : nullptr;

	if (!AIPawn) return EBTNodeResult::Failed;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(AIPawn, 0);

	if (!PlayerPawn) return EBTNodeResult::Failed;

	UAnimInstance* AnimInstance = AIPawn->WolfMesh->GetAnimInstance();

	if (!AnimInstance) return EBTNodeResult::Failed;

	if (AnimInstance && HowlingMontage)
	{
		UAISense_Hearing::ReportNoiseEvent(
			AIPawn->GetWorld(),
			AIPawn->GetActorLocation(),
			1.0f,
			AIPawn,
			0.0f,
			FName("Howling"));

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &UBTT_Howling::OnMontageEnded, &OwnerComp);


		if (HowlSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, HowlSound, PlayerPawn->GetActorLocation());
		}
		AnimInstance->Montage_Play(HowlingMontage);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, HowlingMontage);

		return EBTNodeResult::InProgress;
	}


	return EBTNodeResult::Failed;
}

void UBTT_Howling::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp)
{
	if (Montage == HowlingMontage && OwnerComp)
	{
		FinishLatentTask(*OwnerComp, bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
	}
}