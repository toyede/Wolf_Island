// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/Wolf/BTT_ThrowStone.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Actors/StoneProjectile.h"

EBTNodeResult::Type UBTT_ThrowStone::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AICon = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());

	AIPawn = AICon ? Cast<ACharacter>(AICon->GetPawn()) : nullptr;

	if (!AIPawn) return EBTNodeResult::Failed;

        AnimInstance->Montage_Play(ThrowStoneMontage);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, ThrowStoneMontage);

        if (ThrowSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, ThrowSound, PlayerPawn->GetActorLocation());
        }
        return EBTNodeResult::InProgress;
    }
    return EBTNodeResult::Failed;
}

void UBTT_ThrowStone::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp)
{
	if (Montage == ThrowStoneMontage && OwnerComp && !bInterrupted)
	{
		FinishLatentTask(*OwnerComp, bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
	}
}