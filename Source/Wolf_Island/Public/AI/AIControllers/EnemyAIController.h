// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/AIControllers/EnemyAIcontrollerbase.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyAIController.generated.h"

class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;
class UAISenseConfig_Scent;
class AEnemyAIBase;

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	None UMETA(DisplayName = "None"),
	Passive UMETA(DisplayName = "Passive"),
	Combat UMETA(DisplayName = "Combat"),
	Dead UMETA(DisplayName = "Dead"),
	Frozen UMETA(DisplayName = "Frozen"),
	Investigating UMETA(DisplayName = "Investigating"),
};

UCLASS()
class WOLF_ISLAND_API AEnemyAIController : public AEnemyAIControllerBase
{
	GENERATED_BODY()

public:
	AEnemyAIController();

	// Blackboard Keys
	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
	FName EnemyFormKey = TEXT("Form");

	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
	FName PointOfInterestKey = TEXT("PointOfInterest");

	// Current State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|State")
	EEnemyState EnemyState;

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	void SetEnemyState(EEnemyState NewState);

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<AEnemyAIBase> ControlledEnemy;

	
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void HandleSight(AActor* Actor, const FAIStimulus& Stimulus);
	void HandleDamage(AActor* Actor, const FAIStimulus& Stimulus);
	void HandleHearing(AActor* Actor, const FAIStimulus& Stimulus);
	void HandleScent(const FAIStimulus& Stimulus);

	bool ShouldSwitchTarget(AActor* Newtarget) const;
	bool IsTargetValid(AActor* Target) const;

	void OnEnterState(EEnemyState NewState);
	void OnExitState(EEnemyState OldState);

	// 타이머
	FTimerHandle HearingReactTimer;
	FTimerHandle ForgottenCheckTimer;

	// 퍼셉션
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Damage> DamageConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Scent> ScentConfig;	

	void BindCharacterEvents();

public:
	// Patrol
	UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
	void MoveToNextRoute();
};