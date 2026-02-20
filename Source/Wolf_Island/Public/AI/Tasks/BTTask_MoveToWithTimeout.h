// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTTask_MoveToWithTimeout.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UBTTask_MoveToWithTimeout : public UBTTask_MoveTo
{
	GENERATED_BODY()
	
public:
    UBTTask_MoveToWithTimeout();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, Category = "Timeout")
    float TimeoutDuration = 3.f;

private:
    float ElapsedTime = 0.f;
};
