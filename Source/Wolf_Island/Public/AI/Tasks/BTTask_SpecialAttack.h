// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SpecialAttack.generated.h"

class AEnemyAIBoss;

UCLASS()
class WOLF_ISLAND_API UBTTask_SpecialAttack : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_SpecialAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

private:
	UFUNCTION()
	void OnSpecialAttackEnd();

	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
	TWeakObjectPtr<AEnemyAIBoss> CachedBoss;
};
