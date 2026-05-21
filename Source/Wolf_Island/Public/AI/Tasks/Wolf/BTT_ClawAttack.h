// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Animation/AnimMontage.h"
#include "AI/AIControllers/EnemyAIController.h"
#include "BTT_ClawAttack.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UBTT_ClawAttack : public UBTTaskNode
{
	GENERATED_BODY()

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp);

	// [Refactor] 외부 참조 포인터: raw 포인터 대신 TWeakObjectPtr로 전환 (소유권 없는 외부 참조)
	TWeakObjectPtr<AEnemyAIController> AICon;
	TWeakObjectPtr<ACharacter> AIPawn;

	bool SphereTraceSingle(
		const FVector Start, const FVector End, float Radius,
		const TArray<TEnumAsByte<ECollisionChannel>>& Channels,
		FHitResult& OutHit, const TArray<AActor*>& ActorsToIgnore,
		bool bDrawDebug) const;
public:
	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* ClawAttackMontage;
};
