// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "EnemyAIBossController.generated.h"

UENUM(BlueprintType)
enum class EBossState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Move UMETA(DisplayName = "Move"),
	ThrowAttack UMETA(DisplayName = "ThrowAttack"),
	Rush UMETA(DisplayName = "Rush"),
	SummonStatue UMETA(DisplayName = "SummonStatue"),
	SummonAltar UMETA(DisplayName = "SummonAltar"),
	Attack UMETA(DisplayName = "Attack"),
	Groggy UMETA(DisplayName = "Groggy"),
	Dead UMETA(DisplayName = "Dead"),
	Stun UMETA(DisplayName = "Stun")
};


UCLASS()
class WOLF_ISLAND_API AEnemyAIBossController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIBossController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BehaviorTreeAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UBlackboardComponent* BlackboardComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UBehaviorTreeComponent* BehaviorComp;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName AttackTargetKey = "AttackTarget";

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName BossStateKey = "State";

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	EBossState BossState;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	APawn* AttackTarget;

	UFUNCTION(BlueprintCallable)
	void SetNewState(EBossState NewState);

	UFUNCTION(BlueprintCallable)
	void SetRandomNewState();
};

