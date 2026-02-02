// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateCombatRange.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UBTService_UpdateCombatRange : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateCombatRange();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float CloseRangeEnterDistance = 1500.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float CloseRangeExitDistance = 1700.f;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsCloseRangeModeKey;
};
