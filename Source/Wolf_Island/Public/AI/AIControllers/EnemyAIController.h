// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/AIControllers/EnemyAIcontrollerbase.h"
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
	Attacking UMETA(DisplayName = "Attacking"),
	Dead UMETA(DisplayName = "Dead"),
	Frozen UMETA(DisplayName = "Frozen"),
	Investigating UMETA(DisplayName = "Investigating")
};

UCLASS()
class WOLF_ISLAND_API AEnemyAIController : public AEnemyAIControllerBase
{
	GENERATED_BODY()

public:
	AEnemyAIController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors) override;

	//~ Perception Config
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Damage> DamageConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Scent> ScentConfig;

	

	//~ 감지된 액터
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	TArray<AActor*> KnownSeenActors;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<AEnemyAIBase> ControlledEnemy;

	//~ 타이머
	FTimerHandle HearingReactTimer;
	FTimerHandle ForgottenCheckTimer;

	//~ 내부 함수
	UFUNCTION(BlueprintCallable, Category = "AI|Perception")
	bool CanSenseActor(AActor* Actor, FAIStimulus& OutStimulus);

	UFUNCTION()
	void CheckIfForgottenSeenActor();

	UFUNCTION()
	void HandleForgotActor(AActor* Actor);

public:
	//~ Blackboard Key
	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
	FName EnemyFormKey = TEXT("Form");

	//~ 상태 전환
	UFUNCTION(BlueprintCallable, Category = "AI|State")
	void SetStateAsPassive();

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	void SetStateAsAttacking(AActor* Actor, bool UseLastKnownAttackTarget);

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	void SetStateAsFrozen();

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	void SetStateAsInvestigating(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	void SetStateAsDead();

	//~ Patrol
	UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
	void MoveToNextRoute();

	//~ 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI|State")
	EEnemyState EnemyState;
};