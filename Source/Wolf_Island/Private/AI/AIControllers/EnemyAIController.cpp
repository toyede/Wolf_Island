// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/AIControllers/EnemyAIController.h"

#include "AI/Enemy_Character/EnemyAIBase.h"

#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "AI/Senses/AISenseConfig_Scent.h"

#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Damage.h"
#include "AI/Senses/AISense_Scent.h"

#include "Kismet/GameplayStatics.h"
#include "Actors/PatrolRoute.h"
#include "Components/SplineComponent.h"

AEnemyAIController::AEnemyAIController()
{
	// Sight Config
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1000.f;
	SightConfig->LoseSightRadius = 1700.f;
	SightConfig->PeripheralVisionAngleDegrees = 70.f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->SetMaxAge(5.0f);

	// Hearing Config
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 2000.f;
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->SetMaxAge(2.0f);

	// Damage Config
	DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
	DamageConfig->SetMaxAge(1.0f);

	// Scent Config
	ScentConfig = CreateDefaultSubobject<UAISenseConfig_Scent>(TEXT("ScentConfig"));
	ScentConfig->SetMaxAge(5.0f);

	// Perception에 등록
	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->ConfigureSense(*HearingConfig);
	AIPerceptionComp->ConfigureSense(*DamageConfig);
	AIPerceptionComp->ConfigureSense(*ScentConfig);

	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledEnemy = Cast<AEnemyAIBase>(InPawn);

	if (ControlledEnemy)
	{
		SetStateAsPassive();
		ControlledEnemy->ChangeForm(ControlledEnemy->EnemyForm);
	}

	// Forgotten Actor 체크 타이머
	GetWorld()->GetTimerManager().SetTimer(
		ForgottenCheckTimer,
		this,
		&AEnemyAIController::CheckIfForgottenSeenActor,
		0.5f,
		true
	);
}

void AEnemyAIController::OnUnPossess()
{
	Super::OnUnPossess();

	GetWorld()->GetTimerManager().ClearTimer(ForgottenCheckTimer);
	GetWorld()->GetTimerManager().ClearTimer(HearingReactTimer);
}

void AEnemyAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		return;
	}

	for (AActor* Actor : UpdatedActors)
	{
		FAIStimulus Stimulus;
		if (!CanSenseActor(Actor, Stimulus))
		{
			continue;
		}

		// Sight
		if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>() && Actor == PlayerPawn)
		{
			KnownSeenActors.AddUnique(Actor);
			SetStateAsAttacking(Actor, false);
		}
		// Hearing (Howling)
		else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>() && Stimulus.Tag == FName("Howling"))
		{
			float RandomDelay = FMath::RandRange(1.0f, 2.0f);

			GetWorld()->GetTimerManager().SetTimer(
				HearingReactTimer,
				FTimerDelegate::CreateLambda([this, Actor]()
					{
						SetStateAsAttacking(Actor, false);
					}),
				RandomDelay,
				false
			);
		}

		else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
		{
			SetStateAsAttacking(Actor, false);
		}

		// Scent
		else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Scent>() && Actor == PlayerPawn)
		{
			SetStateAsInvestigating(Stimulus.StimulusLocation);
		}
	}
}

bool AEnemyAIController::CanSenseActor(AActor* Actor, FAIStimulus& OutStimulus)
{
	if (!Actor || !AIPerceptionComp)
	{
		return false;
	}

	FActorPerceptionBlueprintInfo Info;
	AIPerceptionComp->GetActorsPerception(Actor, Info);

	for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			OutStimulus = Stimulus;
			return true;
		}
	}

	return false;
}

void AEnemyAIController::CheckIfForgottenSeenActor()
{
	if (!AIPerceptionComp)
	{
		return;
	}

	TArray<AActor*> KnownPerceived;
	AIPerceptionComp->GetKnownPerceivedActors(UAISense_Sight::StaticClass(), KnownPerceived);

	for (AActor* Actor : KnownSeenActors)
	{
		if (!KnownPerceived.Contains(Actor))
		{
			HandleForgotActor(Actor);
		}
	}
}

void AEnemyAIController::HandleForgotActor(AActor* Actor)
{
	KnownSeenActors.Remove(Actor);

	if (Actor == AttackTarget)
	{
		SetStateAsPassive();
	}
}

void AEnemyAIController::SetStateAsPassive()
{
	AttackTarget = nullptr;
	EnemyState = EEnemyState::Passive;

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsEnum(StateKey, static_cast<uint8>(EnemyState));
		BB->ClearValue(AttackTargetKey);
	}
}

void AEnemyAIController::SetStateAsAttacking(AActor* Actor, bool UseLastKnownAttackTarget)
{
	if (EnemyState == EEnemyState::Dead)
	{
		return;
	}

	AActor* NewAttackTarget;

	if (UseLastKnownAttackTarget && Actor)
	{
		NewAttackTarget = Actor;
	}
	else
	{
		NewAttackTarget = AttackTarget;
	}

	if (!NewAttackTarget)
	{
		SetStateAsPassive();
		return;
	}

	EnemyState = EEnemyState::Attacking;

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsEnum(StateKey, static_cast<uint8>(EnemyState));
		BB->SetValueAsObject(AttackTargetKey, NewAttackTarget);
	}

	AttackTarget = NewAttackTarget;
}

void AEnemyAIController::SetStateAsFrozen()
{

	EnemyState = EEnemyState::Frozen;

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsEnum(StateKey, static_cast<uint8>(EnemyState));
	}
}

void AEnemyAIController::SetStateAsInvestigating(FVector Location)
{
	EnemyState = EEnemyState::Investigating;

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		//BB->SetValueAsVector()
	}
}

void AEnemyAIController::SetStateAsDead()
{

	AttackTarget = nullptr;
	EnemyState = EEnemyState::Dead;

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsEnum(StateKey, static_cast<uint8>(EnemyState));
		BB->ClearValue(AttackTargetKey);
	}
}

void AEnemyAIController::MoveToNextRoute()
{
	if (!ControlledEnemy || !ControlledEnemy->AssignedPatrolRoute)
	{
		return;
	}

	int32 NextIndex = ControlledEnemy->GetNextPoint();
	USplineComponent* Spline = ControlledEnemy->AssignedPatrolRoute->SplinePoints;

	if (Spline)
	{
		FVector TargetLocation = Spline->GetLocationAtSplinePoint(
			NextIndex,
			ESplineCoordinateSpace::World
		);

		MoveToLocation(TargetLocation);
	}
}