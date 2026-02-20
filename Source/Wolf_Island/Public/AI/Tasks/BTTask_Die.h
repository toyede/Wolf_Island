// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Die.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UBTTask_Die : public UBTTaskNode
{
	GENERATED_BODY()

	UBTTask_Die();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
