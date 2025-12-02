// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Animation/AnimMontage.h"
#include "AI/EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "BTT_Howling.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UBTT_Howling : public UBTTaskNode
{
	GENERATED_BODY()
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp);

	AEnemyAIController* AICon;

	class AEnemyAIBase* AIPawn;

public:
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* HowlingMontage;

	UPROPERTY(EditAnywhere, Category = "Trap")
	USoundBase* HowlSound;
};
