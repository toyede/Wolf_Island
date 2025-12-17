// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/AIControllers/EnemyAIBossController.h"

#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Perception/AISenseConfig_Sight.h"

#include "Perception/AISense_Sight.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

AEnemyAIBossController::AEnemyAIBossController()
{
	// Sight Config
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 2000.f;
	SightConfig->LoseSightRadius = 2500.f;
	SightConfig->PeripheralVisionAngleDegrees = 90.f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->SetMaxAge(10.0f);

	// Perception에 등록
	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AEnemyAIBossController::BeginPlay()
{
	Super::BeginPlay();

	InitializeAttackTarget();
}

void AEnemyAIBossController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	SetNewState(EBossState::Idle);
	InitializeAttackTarget();
}

void AEnemyAIBossController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	if (!CurrentTarget)
	{
		CurrentTarget = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	}

	for (AActor* Actor : UpdatedActors)
	{
		if (Actor != CurrentTarget)
		{
			continue;
		}

		FActorPerceptionBlueprintInfo Info;
		AIPerceptionComp->GetActorsPerception(Actor, Info);

		for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
		{
			if (Stimulus.WasSuccessfullySensed() &&
				Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
			{
				if (UBlackboardComponent* BB = GetBlackboardComponent())
				{
					BB->SetValueAsObject(AttackTargetKey, CurrentTarget);
				}
				break;
			}
		}
	}
}

void AEnemyAIBossController::SetNewState(EBossState NewState)
{
	BossState = NewState;

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsEnum(StateKey, static_cast<uint8>(BossState));
	}
}

void AEnemyAIBossController::SetRandomNewState()
{
	TArray<EBossState> Candidates = {
		EBossState::Idle,
		EBossState::Move,
		EBossState::ThrowAttack,
		EBossState::Rush,
		EBossState::SummonStatue,
		EBossState::SummonAltar,
		EBossState::Attack
	};

	int32 Index = FMath::RandRange(0, Candidates.Num() - 1);
	SetNewState(Candidates[Index]);
}

EBossState AEnemyAIBossController::SetStateAsGroggy()
{
	if (BossState == EBossState::Rush)
	{
		SetNewState(EBossState::Groggy);
		return EBossState::Groggy;
	}

	return BossState;
}

EBossState AEnemyAIBossController::SetStateAsStun()
{
	SetNewState(EBossState::Stun);
	return EBossState::Stun;
}

void AEnemyAIBossController::InitializeAttackTarget()
{
	if (CurrentTarget)
	{
		if (UBlackboardComponent* BB = GetBlackboardComponent())
		{
			BB->SetValueAsObject(AttackTargetKey, CurrentTarget);
		}
	}
}