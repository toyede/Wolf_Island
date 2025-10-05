// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BTD_CheckDistance.generated.h"

UCLASS()
class WOLF_ISLAND_API UBTD_CheckDistance : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTD_CheckDistance();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

public:
	UPROPERTY(EditAnywhere, Category = "Player")
	FBlackboardKeySelector PlayerKey;

	UPROPERTY(EditAnywhere, Category = "Enemy")
	FBlackboardKeySelector EnemyKey;

	UPROPERTY(EditAnywhere, Category = "Distance")
	float MaxDistance;
};