// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/StoneProjectile.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "BTT_ThrowStone.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UBTT_ThrowStone : public UBTTaskNode
{
	GENERATED_BODY()

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp);

public:
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* ThrowStoneMontage;

protected:
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AStoneProjectile> ProjectileClass;
};
