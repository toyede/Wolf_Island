// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_WeightedRandom.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UBTD_WeightedRandom : public UBTDecorator
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Random")
	float Probability = 0.5f;

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override
	{
		const float RandomValue = FMath::FRand();
		return RandomValue < Probability;
	}
	
};
