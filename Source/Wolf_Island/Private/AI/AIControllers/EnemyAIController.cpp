// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/AIControllers/EnemyAIController.h"

#include "AI/Enemy_Character/EnemyAIBase.h"

#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
//#include "AI/Senses/AISenseConfig_Scent.h"

#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Damage.h"
//#include "AI/Senses/AISense_Scent.h"

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
	/*ScentConfig = CreateDefaultSubobject<UAISenseConfig_Scent>(TEXT("ScentConfig"));
	ScentConfig->SetMaxAge(5.0f);*/

	// Perception에 등록
	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->ConfigureSense(*HearingConfig);
	AIPerceptionComp->ConfigureSense(*DamageConfig);
	/*AIPerceptionComp->ConfigureSense(*ScentConfig);*/

	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (BehaviorComp)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}

	ControlledEnemy = Cast<AEnemyAIBase>(InPawn);
	if (ControlledEnemy)
	{
		SetEnemyState(EEnemyState::Passive);

		// [이벤트 바인딩] 캐릭터 델리게이트 (공격 종료 등)
		BindCharacterEvents();
	}

	// [핵심 변경] Perception Component의 델리게이트에 바인딩
	// 배열이 아니라 개별 타겟/자극 단위로 호출됨
	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetPerceptionUpdated);
	}
}

void AEnemyAIController::OnUnPossess()
{
	Super::OnUnPossess();

	// 바인딩 해제
	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.RemoveDynamic(this, &AEnemyAIController::OnTargetPerceptionUpdated);
	}

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void AEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (EnemyState == EEnemyState::Dead) return;

	TSubclassOf<UAISense> SensedClass = UAIPerceptionSystem::GetSenseClassForStimulus(GetWorld(), Stimulus);

	if (SensedClass == UAISense_Sight::StaticClass())
	{
		HandleSight(Actor, Stimulus);
	}
	else if (SensedClass == UAISense_Damage::StaticClass())
	{
		HandleDamage(Actor, Stimulus);
	}
	else if (SensedClass == UAISense_Hearing::StaticClass())
	{
		HandleHearing(Actor, Stimulus);
	}
	/*else if (SensedClass == UAISense_Scent::StaticClass())
	{
		HandleScent(Stimulus);
	}*/
}

void AEnemyAIController::HandleSight(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed())
	{
		if (EnemyState == EEnemyState::Combat && Actor == AttackTarget)
		{
			GetWorld()->GetTimerManager().SetTimer(LineOfSightTimer, [this]()
				{
					SetEnemyState(EEnemyState::Passive);
				}, 3.0f, false);
		}
		return;
	}

	APawn* SensedPawn = Cast<APawn>(Actor);
	if (!SensedPawn || !SensedPawn->IsPlayerControlled()) return;

	if (Actor->ActorHasTag(FName("Invisible")) || Actor->ActorHasTag(FName("Werewolf")))
	{
		SetEnemyState(EEnemyState::Passive);
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(LineOfSightTimer);

	if (ShouldSwitchTarget(Actor))
	{
		AttackTarget = Actor;

		if (EnemyState != EEnemyState::Combat)
		{
			SetEnemyState(EEnemyState::Combat);
		}
	}
}

void AEnemyAIController::HandleDamage(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed()) return;
	if (IsFriendlyAggroSource(Actor)) return;
	if (Actor->ActorHasTag(FName("Invisible")) || Actor->ActorHasTag(FName("Werewolf")))
	{
		SetEnemyState(EEnemyState::Passive);
		return;
	}
	AttackTarget = Actor;
	SetEnemyState(EEnemyState::Combat);
}

void AEnemyAIController::HandleHearing(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed()) return;
	if (Stimulus.Tag != FName("Howling")) return;

	if (EnemyState == EEnemyState::Combat) return;

	// Howling 연계는 "플레이어 타겟 공유"만 허용한다.
	AActor* SharedPlayerTarget = nullptr;

	// 1) 소리를 직접 낸 대상이 플레이어면 그대로 사용
	if (APawn* SensedPawn = Cast<APawn>(Actor))
	{
		if (SensedPawn->IsPlayerControlled())
		{
			SharedPlayerTarget = Actor;
		}
	}

	// 2) 소리를 낸 대상이 Enemy면, 그 Enemy의 현재 AttackTarget(플레이어)만 공유
	if (!SharedPlayerTarget)
	{
		if (AEnemyAIBase* SourceEnemy = Cast<AEnemyAIBase>(Actor))
		{
			if (AEnemyAIController* SourceController = Cast<AEnemyAIController>(SourceEnemy->GetController()))
			{
				AActor* CandidateTarget = SourceController->AttackTarget;
				if (APawn* CandidatePawn = Cast<APawn>(CandidateTarget))
				{
					if (CandidatePawn->IsPlayerControlled())
					{
						SharedPlayerTarget = CandidateTarget;
					}
				}
			}
		}
	}

	// 플레이어 타겟이 없으면 전투 전환하지 않음 (늑대->늑대 타겟 방지)
	if (!IsValid(SharedPlayerTarget)) return;

	float RandomDelay = FMath::RandRange(1.0f, 2.0f);
	FTimerDelegate TimerDel;
	TimerDel.BindWeakLambda(this, [this, SharedPlayerTarget]()
		{
			if (IsValid(SharedPlayerTarget) && EnemyState != EEnemyState::Combat && EnemyState != EEnemyState::Dead)
			{
				AttackTarget = SharedPlayerTarget;
				SetEnemyState(EEnemyState::Combat);
			}
		});

	GetWorld()->GetTimerManager().SetTimer(HearingReactTimer, TimerDel, RandomDelay, false);
}

bool AEnemyAIController::IsFriendlyAggroSource(AActor* SourceActor) const
{
	if (!IsValid(SourceActor))
	{
		return false;
	}

	// Direct enemy actor
	if (SourceActor->IsA<AEnemyAIBase>())
	{
		return true;
	}

	// Projectile/child actor owned by an enemy
	if (AActor* OwnerActor = SourceActor->GetOwner())
	{
		if (OwnerActor->IsA<AEnemyAIBase>())
		{
			return true;
		}
	}

	// Instigator pawn is an enemy
	if (APawn* InstigatorPawn = SourceActor->GetInstigator())
	{
		if (InstigatorPawn->IsA<AEnemyAIBase>())
		{
			return true;
		}
	}

	// Instigator controller controls an enemy pawn
	if (AController* InstigatorController = SourceActor->GetInstigatorController())
	{
		if (APawn* InstigatorPawn = InstigatorController->GetPawn())
		{
			if (InstigatorPawn->IsA<AEnemyAIBase>())
			{
				return true;
			}
		}
	}

	return false;
}

//void AEnemyAIController::HandleScent(const FAIStimulus& Stimulus)
//{
//	if (!Stimulus.WasSuccessfullySensed()) return;
//
//	// 평화롭거나 조사 중일 때만 냄새 반응
//	if (EnemyState == EEnemyState::Passive || EnemyState == EEnemyState::Investigating)
//	{
//		// 냄새 위치 저장
//		if (UBlackboardComponent* BB = GetBlackboardComponent())
//		{
//			BB->SetValueAsVector(PointOfInterestKey, Stimulus.StimulusLocation);
//		}
//		SetEnemyState(EEnemyState::Investigating);
//	}
//}

bool AEnemyAIController::ShouldSwitchTarget(AActor* NewTarget) const
{
	if (!IsTargetValid(AttackTarget)) return true;

	if (EnemyState == EEnemyState::Combat)
	{
		return false;
	}
	return true;
}

bool AEnemyAIController::IsTargetValid(AActor* Target) const
{
	return IsValid(Target);
}

// ============================================================================
// [3] State Management (Switch-Case Centralized)
// ============================================================================

void AEnemyAIController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AEnemyAIController, EnemyState);
}

void AEnemyAIController::SetEnemyState(EEnemyState NewState)
{
	if (!HasAuthority()) return;
	if (EnemyState == NewState) return;

	OnExitState(EnemyState); // 이전 상태 정리
	EnemyState = NewState;   // 상태 변경

	// 블랙보드 Enum 업데이트
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsEnum(StateKey, static_cast<uint8>(EnemyState));
	}

	OnEnterState(NewState);  // 새 상태 진입

	OnEnemyStateChanged.Broadcast(NewState);
}

void AEnemyAIController::OnRep_State()
{
	AEnemyAIBase* Enemy = Cast<AEnemyAIBase>(GetPawn());
	
	if (!Enemy) return;

	Enemy->ApplySpeedByState(EnemyState);
}

void AEnemyAIController::OnEnterState(EEnemyState NewState)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB) return;

	switch (NewState)
	{
	case EEnemyState::Passive:
		AttackTarget = nullptr;
		BB->ClearValue(AttackTargetKey);
		BB->SetValueAsBool(TEXT("bIsHalfHP"), false);
		break;

	case EEnemyState::Combat:
		if (IsTargetValid(AttackTarget))
		{
			BB->SetValueAsObject(AttackTargetKey, AttackTarget);
		}
		else
		{
			SetEnemyState(EEnemyState::Passive); // 타겟 유효성 재확인
		}
		break;

	case EEnemyState::Investigating:
		AttackTarget = nullptr;
		BB->ClearValue(AttackTargetKey);
		break;

	case EEnemyState::Dead:
	case EEnemyState::Frozen:
		StopMovement();
		break;
	}
}

void AEnemyAIController::OnExitState(EEnemyState OldState)
{
	// 필요 시 구현 (예: 타이머 클리어 등)
}

// ============================================================================
// [4] Event Binding (Task Completion)
// ============================================================================

void AEnemyAIController::BindCharacterEvents()
{
	// 캐릭터 스크립트에 델리게이트가 있다고 가정 (예: OnAttackMontageEnded)
	// if (ControlledEnemy)
	// {
	//    ControlledEnemy->OnAttackEnded.AddDynamic(this, &AEnemyAIController::OnCharacterAttackFinished);
	// }
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
