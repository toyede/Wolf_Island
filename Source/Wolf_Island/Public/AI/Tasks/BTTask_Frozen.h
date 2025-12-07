// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Frozen.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UBTTask_Frozen : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Frozen();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp);
};
