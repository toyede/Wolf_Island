// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_ClawAttack.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UBTT_ClawAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_ClawAttack();

    UPROPERTY(EditAnywhere, Category = "AI")
    float AttackRange = 200.0f;

protected:
    // Task 실행 시 호출
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory
    ) override;
};
