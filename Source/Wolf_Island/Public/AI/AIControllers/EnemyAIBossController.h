// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/AIControllers/EnemyAIcontrollerbase.h"
#include "EnemyAIBossController.generated.h"

class UAISenseConfig_Sight;

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
class WOLF_ISLAND_API AEnemyAIBossController : public AEnemyAIControllerBase
{
	GENERATED_BODY()

public:
	AEnemyAIBossController();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors) override;

	//~ Perception Config
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	//~ 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI|State")
	EBossState BossState = EBossState::Idle;

	//~ Current Target
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI|Target")
	TObjectPtr<ACharacter> CurrentTarget;

	void InitializeAttackTarget();

public:
	//~ 상태 전환
	UFUNCTION(BlueprintCallable, Category = "AI|State")
	void SetNewState(EBossState NewState);

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	void SetRandomNewState();

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	EBossState SetStateAsGroggy();

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	EBossState SetStateAsStun();

	UFUNCTION(BlueprintPure, Category = "AI|State")
	EBossState GetCurrentState() const { return BossState; }
};