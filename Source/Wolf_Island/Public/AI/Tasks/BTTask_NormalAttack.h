// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_NormalAttack.generated.h"

/**
 * 
 */
class AEnemyAIBase;

UCLASS()
class WOLF_ISLAND_API UBTTask_NormalAttack : public UBTTaskNode
{
	GENERATED_BODY()
	
	UBTTask_NormalAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UFUNCTION()
	void OnAttackFinished();

private:
	TWeakObjectPtr<AEnemyAIBase> CachedEnemy;
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
};
