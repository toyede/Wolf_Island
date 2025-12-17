// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIControllerBase.generated.h"

class UBehaviorTreeComponent;

UCLASS(Abstract)
class WOLF_ISLAND_API AEnemyAIControllerBase : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIControllerBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Target")
	TObjectPtr<AActor> AttackTarget;

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	//~ BehaviorTree
	UPROPERTY(EditAnywhere, Category = "AI|BehaviorTree")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|BehaviorTree")
	TObjectPtr<UBehaviorTreeComponent> BehaviorComp;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
	FName AttackTargetKey = TEXT("AttackTarget");

	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
	FName StateKey = TEXT("State");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;

	UFUNCTION()
	virtual void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
};