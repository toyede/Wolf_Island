// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Animation/AnimMontage.h"
#include "AI/AIControllers/EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "BTT_Howling.generated.h"

/**
 * 
 */
class AEnemyBase;

UCLASS()
class WOLF_ISLAND_API UBTT_Howling : public UBTTaskNode
{
	GENERATED_BODY()
	
	UBTT_Howling();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UFUNCTION()
	void OnHowlingFinished();

private:
	TWeakObjectPtr<AEnemyAIBase> CachedEnemy;
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
};
