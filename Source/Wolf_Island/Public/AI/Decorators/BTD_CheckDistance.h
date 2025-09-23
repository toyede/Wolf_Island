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
	/** 조건 계산 */
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

public:
	/** 거리 체크할 플레이어 */
	UPROPERTY(EditAnywhere, Category = "Player")
	FBlackboardKeySelector PlayerKey;

	/** 보스 (Querier) */
	UPROPERTY(EditAnywhere, Category = "Boss")
	FBlackboardKeySelector BossKey;

	/** 최대 거리 */
	UPROPERTY(EditAnywhere, Category = "Distance")
	float MaxDistance;
};