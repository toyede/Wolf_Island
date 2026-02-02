// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/AIControllers/EnemyAIcontrollerbase.h"
#include "EnemyAIBossController.generated.h"

UENUM(BlueprintType)
enum class EBossState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Combat UMETA(DisplayName = "Combat"),
	Groggy UMETA(DisplayName = "Groggy"),
	Stun UMETA(DisplayName = "Stun"),
	Dead UMETA(DisplayName = "Dead")
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
	virtual void OnUnPossess() override;

	// 초기 상태
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI|State")
	EBossState BossState = EBossState::Combat;

	// 공격 타겟
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AI|Target")
	TObjectPtr<ACharacter> CurrentTarget;

public:
	// 상태 전환
	UFUNCTION(BlueprintCallable, Category = "AI|State")
	void SetNewState(EBossState NewState);

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	EBossState SetStateAsGroggy();

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	EBossState SetStateAsStun();

	UFUNCTION(BlueprintPure, Category = "AI|State")
	EBossState GetCurrentState() const { return BossState; }

	UFUNCTION()
	void HandlePhaseChanged(int32 NewPhase);
};