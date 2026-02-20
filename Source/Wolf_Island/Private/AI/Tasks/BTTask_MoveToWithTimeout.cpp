// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_MoveToWithTimeout.h"

UBTTask_MoveToWithTimeout::UBTTask_MoveToWithTimeout()
{
    NodeName = TEXT("Move To With Timeout");
    bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_MoveToWithTimeout::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ElapsedTime = 0.f;
    return Super::ExecuteTask(OwnerComp, NodeMemory);
}

void UBTTask_MoveToWithTimeout::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    ElapsedTime += DeltaSeconds;

    if (ElapsedTime >= TimeoutDuration)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
}

