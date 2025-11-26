// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIBossController.generated.h"

class UBehaviorTree;
class UBehaviorTreeComponent;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AttackTarget")
	ACharacter* Player;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI|State")
	EBossState BossState = EBossState::Idle;

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	void SetNewState(EBossState NewState);

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	void SetRandomNewState();

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	EBossState SetStateAsGroggy();

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	EBossState SetStateAsStun();

	UFUNCTION()
	void SetAttackTarget();

	UFUNCTION(BlueprintPure, Category = "AI|State")
	EBossState GetCurrentState() const { return BossState; }

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTreeComponent> BehaviorComp;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
	FName AttackTargetKey = TEXT("AttackTarget");

	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
	FName BossStateKey = TEXT("State");
};

