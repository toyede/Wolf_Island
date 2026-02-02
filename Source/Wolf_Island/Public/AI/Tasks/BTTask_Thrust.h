// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Thrust.generated.h"

class AEnemyAIBoss;

UCLASS()
class WOLF_ISLAND_API UBTTask_Thrust : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_Thrust();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

private:
	UFUNCTION()
	void OnThrustEnd();
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
	TWeakObjectPtr<AEnemyAIBoss> CachedBoss;
};
