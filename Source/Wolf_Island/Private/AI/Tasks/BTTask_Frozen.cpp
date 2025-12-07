// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_Frozen.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "AI/Enemy_Character/EnemyAIBase.h"
#include "AI/AIControllers/EnemyAIController.h"

UBTTask_Frozen::UBTTask_Frozen()
{
	NodeName = TEXT("Frozen");
}

EBTNodeResult::Type UBTTask_Frozen::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon)
    {
        return EBTNodeResult::Failed;
    }

    AEnemyAIBase* Pawn = Cast<AEnemyAIBase>(AICon->GetPawn());
    if (!Pawn)
    {
        return EBTNodeResult::Failed;
    }

    
    // 몽타주 가져오기
    UAnimMontage* FrozenMontage_Native = Pawn->FrozenMontage_Native;
    UAnimMontage* FrozenMontage_Wolf = Pawn->FrozenMontage_Wolf;

    if (!FrozenMontage_Native || !FrozenMontage_Wolf)
    {
        return EBTNodeResult::Failed;
    }

    UAnimInstance* AnimInstance = Pawn->GetMesh()->GetAnimInstance();
    if (AnimInstance)
    {
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &UBTTask_Frozen::OnMontageEnded, &OwnerComp);
        if (Pawn->bIsHuman)
        {
            AnimInstance->Montage_SetEndDelegate(EndDelegate, FrozenMontage_Native);
        }
        else
        {
            AnimInstance->Montage_SetEndDelegate(EndDelegate, FrozenMontage_Wolf);
        }
        
        return EBTNodeResult::InProgress;
    }

    return EBTNodeResult::Succeeded;
}

// BTTask_Frozen.cpp - OnMontageEnded에 Dead 체크 추가
void UBTTask_Frozen::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp)
{
    if (!OwnerComp)
    {
        return;
    }

    if (bInterrupted)
    {
        FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
        return;
    }


    FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
}