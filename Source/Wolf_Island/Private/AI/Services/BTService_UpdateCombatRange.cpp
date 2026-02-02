// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Services/BTService_UpdateCombatRange.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_UpdateCombatRange::UBTService_UpdateCombatRange()
{
    NodeName = "Update Combat Range";
    Interval = 0.2f;
    RandomDeviation = 0.05f;
}

void UBTService_UpdateCombatRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    AAIController* AIC = OwnerComp.GetAIOwner();

    if (!BB || !AIC || !AIC->GetPawn()) return;

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));
    if (!Target) return;

    const float Distance = FVector::Dist(AIC->GetPawn()->GetActorLocation(), Target->GetActorLocation());
    const bool bCurrentlyCloseRange = BB->GetValueAsBool(IsCloseRangeModeKey.SelectedKeyName);

    bool bNewCloseRange = bCurrentlyCloseRange;

    if (bCurrentlyCloseRange)
    {
        // 근접 모드 중 → 이탈 거리 초과해야 원거리로
        if (Distance > CloseRangeExitDistance)
        {
            bNewCloseRange = false;
        }
    }
    else
    {
        // 원거리 모드 중 → 진입 거리 미만이어야 근접으로
        if (Distance < CloseRangeEnterDistance)
        {
            bNewCloseRange = true;
        }
    }

    BB->SetValueAsBool(IsCloseRangeModeKey.SelectedKeyName, bNewCloseRange);
}
