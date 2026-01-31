// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AIControllers/AnimalController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "AI/Senses/AISenseConfig_Scent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Damage.h"
#include "AI/Senses/AISense_Scent.h"

AAnimalController::AAnimalController()
{
	BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));

	// Sight Config
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 600.f;
	SightConfig->LoseSightRadius = 700.f;
	SightConfig->PeripheralVisionAngleDegrees = 60.f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->SetMaxAge(5.0f);

	// Hearing Config
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 800.f;
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

void AAnimalController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAnimalController, AnimalState);
}

void AAnimalController::SetAnimalState(EAnimalState NewState)
{
	if (!HasAuthority()) return;
	if (AnimalState == NewState) return;
	AnimalState = NewState;
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		//BB->SetValueAsEnum(StateKey, static_cast<uint8>(AnimalState));
	}
}

void AAnimalController::OnRep_State()
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsEnum(StateKey, static_cast<uint8>(AnimalState));
	}
}

void AAnimalController::BeginPlay()
{
	Super::BeginPlay();
}

void AAnimalController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}

	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AAnimalController::OnTargetPerceptionUpdated);
	}
}

void AAnimalController::OnUnPossess()
{
	Super::OnUnPossess();

	if (BehaviorTreeAsset)
	{
		BehaviorComp->StopTree(EBTStopMode::Safe);
	}

	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.RemoveDynamic(this, &AAnimalController::OnTargetPerceptionUpdated);
	}
}

void AAnimalController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (AnimalState == EAnimalState::Dead) return;

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
	else if (SensedClass == UAISense_Scent::StaticClass())
	{
		HandleScent(Stimulus);
	}
}


void AAnimalController::HandleSight(AActor* Actor, const FAIStimulus& Stimulus)
{
	//// 1. 시야 소실
	//if (!Stimulus.WasSuccessfullySensed())
	//{
	//	// 플레이어를 놓쳤다고 바로 포기하지 않고, 
	//	// "3초 뒤에도 안 보이면 포기해라"라고 예약을 겁니다.
	//	if (AnimalState == EAnimalState::Combat && Actor == AttackTarget)
	//	{
	//		GetWorld()->GetTimerManager().SetTimer(LineOfSightTimer, [this]()
	//			{
	//				// 3초 뒤 실행될 코드:
	//				// 여전히 타겟이 안 보인다면(혹은 거리가 멀다면) 포기
	//				SetAnimalState(EAnimalState::Passive);
	//			}, 3.0f, false); // 3.0f는 '기억 지속 시간'
	//	}
	//	return;
	//}

	//// 2. 시야 감지 & 플레이어 확인
	//APawn* SensedPawn = Cast<APawn>(Actor);
	//if (!SensedPawn || !SensedPawn->IsPlayerControlled()) return;

	//GetWorld()->GetTimerManager().ClearTimer(LineOfSightTimer);

	//// 3. 타겟 전환이 필요할 때만 업데이트
	//if (ShouldSwitchTarget(Actor))
	//{
	//	AttackTarget = Actor;

	//	// 이미 Combat이면 State 재설정 안 함
	//	if (EnemyState != EEnemyState::Combat)
	//	{
	//		SetEnemyState(EEnemyState::Combat);
	//	}
	//}
}

void AAnimalController::HandleDamage(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed()) return;

	// 동물은 도망가기만 함
	SetAnimalState(EAnimalState::Escaping);

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsEnum(StateKey, static_cast<uint8>(AnimalState));
		BB->SetValueAsObject(TargetKey, Actor);
	}
}

void AAnimalController::HandleHearing(AActor* Actor, const FAIStimulus& Stimulus)
{
	//if (!Stimulus.WasSuccessfullySensed()) return;
	//if (Stimulus.Tag != FName("Howling")) return;

	//// 이미 교전 중이면 소리 무시
	//if (AnimalState == EAnimalState::Combat) return;

	//// 반응 딜레이
	//float RandomDelay = FMath::RandRange(1.0f, 2.0f);
	//FTimerDelegate TimerDel;
	//TimerDel.BindWeakLambda(this, [this, Actor]()
	//	{
	//		// 딜레이 후에도 타겟이 유효하고 아직 교전 중이 아니라면 공격 시작
	//		if (IsValid(Actor) && EnemyState != EEnemyState::Combat && EnemyState != EEnemyState::Dead)
	//		{
	//			AttackTarget = Actor;
	//			SetEnemyState(EEnemyState::Combat);
	//		}
	//	});

	//GetWorld()->GetTimerManager().SetTimer(HearingReactTimer, TimerDel, RandomDelay, false);
}

void AAnimalController::HandleScent(const FAIStimulus& Stimulus)
{
	//if (!Stimulus.WasSuccessfullySensed()) return;

	//// 평화롭거나 조사 중일 때만 냄새 반응
	//if (EnemyState == EEnemyState::Passive || EnemyState == EEnemyState::Investigating)
	//{
	//	// 냄새 위치 저장
	//	if (UBlackboardComponent* BB = GetBlackboardComponent())
	//	{
	//		BB->SetValueAsVector(PointOfInterestKey, Stimulus.StimulusLocation);
	//	}
	//	SetEnemyState(EEnemyState::Investigating);
	//}
}