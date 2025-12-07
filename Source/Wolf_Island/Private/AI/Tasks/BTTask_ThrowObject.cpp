#include "AI/Tasks/BTTask_ThrowObject.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "AI/Interfaces/EnemyCommonInterface.h" 

UBTTask_ThrowObject::UBTTask_ThrowObject()
{
    NodeName = TEXT("Throw Object");
    bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_ThrowObject::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;

    if (!Pawn || !Pawn->Implements<UEnemyCommonInterface>())
    {
        return EBTNodeResult::Failed;
    }

    UAnimMontage* Montage = IEnemyCommonInterface::Execute_GetThrowMontage(Pawn);
    if (!Montage)
    {
        return EBTNodeResult::Failed;
    }

    IEnemyCommonInterface::Execute_ThrowObject(Pawn);

    ACharacter* Character = Cast<ACharacter>(Pawn);
    if (Character)
    {
        UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(this, &UBTTask_ThrowObject::OnMontageEnded, &OwnerComp);
            AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);

            return EBTNodeResult::InProgress;
        }
    }

    return EBTNodeResult::Succeeded;
}

EBTNodeResult::Type UBTTask_ThrowObject::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;

    if (Pawn && Pawn->Implements<UEnemyCommonInterface>())
    {
        UAnimMontage* Montage = IEnemyCommonInterface::Execute_GetThrowMontage(Pawn);
        ACharacter* Character = Cast<ACharacter>(Pawn);

        if (Character && Montage)
        {
            UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
            if (AnimInstance)
            {
                FOnMontageEnded EmptyDelegate;
                AnimInstance->Montage_SetEndDelegate(EmptyDelegate, Montage);

                if (AnimInstance->Montage_IsPlaying(Montage))
                {
                    AnimInstance->Montage_Stop(0.2f, Montage);
                }
            }
        }
    }

    return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_ThrowObject::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp)
{
    if (OwnerComp && OwnerComp->GetActiveNode() == this)
    {
        FinishLatentTask(*OwnerComp, bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
    }
}