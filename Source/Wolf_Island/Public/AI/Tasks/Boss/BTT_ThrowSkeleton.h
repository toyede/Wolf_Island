// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/StoneProjectile.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Animation/AnimMontage.h"
#include "AI/AIControllers/EnemyAIBossController.h"
#include "AIController.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "BTT_ThrowSkeleton.generated.h"

UCLASS()
class WOLF_ISLAND_API UBTT_ThrowSkeleton : public UBTTaskNode
{
	GENERATED_BODY()

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp);

public:
	AEnemyAIBossController* AICon;

	ACharacter* AIPawn;
protected:
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* ThrowSkeletonMontage;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AStoneProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* ThrowSound;
};
