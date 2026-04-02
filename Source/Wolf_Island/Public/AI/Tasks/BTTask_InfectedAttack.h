// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_InfectedAttack.generated.h"

class AWerewolfInfected;

UCLASS()
class WOLF_ISLAND_API UBTTask_InfectedAttack : public UBTTaskNode
{
	GENERATED_BODY()
	
	UBTTask_InfectedAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UFUNCTION()
	void OnAttackFinished();

private:
	TWeakObjectPtr<AWerewolfInfected> CachedInfected;
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
};
