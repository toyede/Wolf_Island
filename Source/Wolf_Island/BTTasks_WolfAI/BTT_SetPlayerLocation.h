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
    // Task ���� �� ȣ��
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory
    ) override;

    // �����Ϳ��� ������ ������ Ű
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector PlayerLocationKey;
	
};
