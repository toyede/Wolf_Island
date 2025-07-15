// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_SetPlayerLocation.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UBTT_SetPlayerLocation : public UBTTaskNode
{
	GENERATED_BODY()
public:
    UBTT_SetPlayerLocation();

protected:
    // Task 실행 시 호출
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory
    ) override;

    // 에디터에서 지정할 블랙보드 키
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector PlayerLocationKey;
	
};
