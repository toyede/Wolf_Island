// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "UBTService_UpdateTargetInfo.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UUBTService_UpdateTargetInfo : public UBTService
{
	GENERATED_BODY()
	
public:
	UUBTService_UpdateTargetInfo();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector DistanceKey;;
};
