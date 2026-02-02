// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/AIControllers/EnemyAIBossController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "AI/Enemy_Character/EnemyAIBoss.h"

AEnemyAIBossController::AEnemyAIBossController()
{
}

void AEnemyAIBossController::BeginPlay()
{
	Super::BeginPlay();
}

void AEnemyAIBossController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	SetNewState(EBossState::Combat);

	if (AEnemyAIBoss* Boss = Cast<AEnemyAIBoss>(InPawn))
	{
		Boss->OnPhaseChanged.AddDynamic(this, &AEnemyAIBossController::HandlePhaseChanged);
	}
}

void AEnemyAIBossController::OnUnPossess()
{
	if (AEnemyAIBoss* Boss = Cast<AEnemyAIBoss>(GetPawn()))
	{
		Boss->OnPhaseChanged.RemoveDynamic(this, &AEnemyAIBossController::HandlePhaseChanged);
	}

	Super::OnUnPossess();
}

void AEnemyAIBossController::SetNewState(EBossState NewState)
{
	BossState = NewState;

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsEnum(StateKey, static_cast<uint8>(BossState));
	}
}

EBossState AEnemyAIBossController::SetStateAsGroggy()
{
	/*if (BossState == EBossState::Rush)
	{
		SetNewState(EBossState::Groggy);
		return EBossState::Groggy;
	}*/

	return BossState;
}

EBossState AEnemyAIBossController::SetStateAsStun()
{
	SetNewState(EBossState::Stun);
	return EBossState::Stun;
}

void AEnemyAIBossController::HandlePhaseChanged(int32 NewPhase)
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsInt(TEXT("CurrentPhase"), NewPhase);
	}
}