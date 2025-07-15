// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h" //Use BT
#include "WolfAIController.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API AWolfAIController : public AAIController
{
	GENERATED_BODY()

public:
	AWolfAIController();

protected:
	// Possess; Parameter must be APawn
	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	UBehaviorTree* BT_WolfAI;

private:
	// Components used when rumtiming
	UPROPERTY(Transient)
	class UBehaviorTreeComponent* BehaviorTreeComponent;

	UPROPERTY(Transient)
	class UBlackboardComponent* BlackboardComponent;


	
};
