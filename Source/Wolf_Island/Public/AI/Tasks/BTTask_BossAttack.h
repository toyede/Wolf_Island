// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_BossAttack.generated.h"

class AEnemyAIBoss;

UCLASS()
class WOLF_ISLAND_API UBTTask_BossAttack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_BossAttack();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

    UPROPERTY(EditAnywhere, Category = "Attack")
    int32 AttackIndex = 0;

    UPROPERTY(EditAnywhere, Category = "Attack")
	bool bClearTargetAfterAttack = false;

private:
    UFUNCTION()
    void OnBossAttackEnd();

    TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
    TWeakObjectPtr<AEnemyAIBoss> CachedBoss;
};