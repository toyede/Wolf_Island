// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/AIControllers/EnemyAIBossController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "AI/Enemy_Character/EnemyAIBoss.h"

AEnemyAIBossController::AEnemyAIBossController()
{
	bSetControlRotationFromPawnOrientation = false;
}

void AEnemyAIBossController::BeginPlay()
{
	Super::BeginPlay();
}

void AEnemyAIBossController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// SetNewState(EBossState::Combat);

	if (AEnemyAIBoss* Boss = Cast<AEnemyAIBoss>(InPawn))
	{
		Boss->OnPhaseChanged.AddDynamic(this, &AEnemyAIBossController::HandlePhaseChanged);
	}
}

void AEnemyAIBossController::StartBehaviorTree()
{
	SetNewState(EBossState::Combat);
	RunBehaviorTree(BehaviorTreeAsset);
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
	// [Refactor] 델리게이트 콜백 진입부: BlackboardComponent 유효성 체크
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!IsValid(BB))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Refactor] AEnemyAIBossController::HandlePhaseChanged: BlackboardComponent is invalid"));
		return;
	}
	BB->SetValueAsInt(TEXT("CurrentPhase"), NewPhase);
}