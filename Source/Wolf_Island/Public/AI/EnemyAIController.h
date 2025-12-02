// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	None UMETA(DisplayName = "None"),
	Passive UMETA(DisplayName = "Passive"),
	Attacking UMETA(DisplayName = "Attacking"),
	Dead UMETA(DisplayName = "Dead")
};

class AEnemyAIBase;

UCLASS()
class WOLF_ISLAND_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

	virtual void OnUnPossess() override;
	
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BehaviorTreeAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UBlackboardComponent* BlackboardComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UBehaviorTreeComponent* BehaviorComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAIPerceptionComponent* AIPerceptionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAISenseConfig_Hearing* HearingConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAISenseConfig_Damage* DamageConfig;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName AttackTargetKey = "AttackTarget";

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName EnemyStateKey = "State";

	UFUNCTION(BlueprintCallable)
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	//UFUNCTION()
	//void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION(BlueprintCallable)
	bool CanSensedActor(AActor* Actor, FAIStimulus& LastStimulus);

	UFUNCTION(BlueprintCallable)
	void SetStateAsAttacking(AActor* Actor);

	UFUNCTION(BlueprintCallable)
	void CheckIfForgottenSeenActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI")
	TArray<AActor*> KnownSeenActors;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI");
	EEnemyState EnemyState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	AEnemyAIBase* ControlledEnemy;

	FTimerHandle HearingReactTimer;

	FTimerHandle TimerHandle;

	UFUNCTION(BlueprintCallable)
	void HandleForgotActor(AActor* Actor);

	AActor* AttackTarget;

public:
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	FName EnemyFormKey = "Form";

	UFUNCTION()
	void SetStateAsPassive();
};

