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
//#include "AI/Senses/AISenseConfig_Scent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Damage.h"
//#include "AI/Senses/AISense_Scent.h"
#include "AI/Animal/AnimalBase.h"
#include "GameFramework/CharacterMovementComponent.h"

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

	//// Scent Config
	//ScentConfig = CreateDefaultSubobject<UAISenseConfig_Scent>(TEXT("ScentConfig"));
	//ScentConfig->SetMaxAge(5.0f);

	// Perception�� ���
	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->ConfigureSense(*HearingConfig);
	AIPerceptionComp->ConfigureSense(*DamageConfig);
	//AIPerceptionComp->ConfigureSense(*ScentConfig);

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
		BB->SetValueAsEnum(StateKey, static_cast<uint8>(AnimalState));
	}

	if (APawn* ControlledPawn = GetPawn())
	{
		if (AAnimalBase* Animal = Cast<AAnimalBase>(ControlledPawn))
		{
			UCharacterMovementComponent* Movement = Animal->GetCharacterMovement();

			if (NewState == EAnimalState::Escaping)
			{
				Movement->MaxWalkSpeed = EscapeSpeed;
			}
			else
			{
				Movement->MaxWalkSpeed = PatrolSpeed;
			}
		}
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

	// [Refactor] 타이머 정리: UnPossess 시 등록된 모든 타이머 해제
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

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
	//else if (SensedClass == UAISense_Scent::StaticClass())
	//{
	//	HandleScent(Stimulus);
	//}
}


void AAnimalController::HandleSight(AActor* Actor, const FAIStimulus& Stimulus)
{
	//// 1. �þ� �ҽ�
	//if (!Stimulus.WasSuccessfullySensed())
	//{
	//	// �÷��̾ ���ƴٰ� �ٷ� �������� �ʰ�, 
	//	// "3�� �ڿ��� �� ���̸� �����ض�"��� ������ �̴ϴ�.
	//	if (AnimalState == EAnimalState::Combat && Actor == AttackTarget)
	//	{
	//		GetWorld()->GetTimerManager().SetTimer(LineOfSightTimer, [this]()
	//			{
	//				// 3�� �� ����� �ڵ�:
	//				// ������ Ÿ���� �� ���δٸ�(Ȥ�� �Ÿ��� �ִٸ�) ����
	//				SetAnimalState(EAnimalState::Passive);
	//			}, 3.0f, false); // 3.0f�� '��� ���� �ð�'
	//	}
	//	return;
	//}

	//// 2. �þ� ���� & �÷��̾� Ȯ��
	//APawn* SensedPawn = Cast<APawn>(Actor);
	//if (!SensedPawn || !SensedPawn->IsPlayerControlled()) return;

	//GetWorld()->GetTimerManager().ClearTimer(LineOfSightTimer);

	//// 3. Ÿ�� ��ȯ�� �ʿ��� ���� ������Ʈ
	//if (ShouldSwitchTarget(Actor))
	//{
	//	AttackTarget = Actor;

	//	// �̹� Combat�̸� State �缳�� �� ��
	//	if (EnemyState != EEnemyState::Combat)
	//	{
	//		SetEnemyState(EEnemyState::Combat);
	//	}
	//}
}

void AAnimalController::HandleDamage(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed()) return;

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsObject(TargetKey, Actor);
	}
}

void AAnimalController::HandleHearing(AActor* Actor, const FAIStimulus& Stimulus)
{
	//if (!Stimulus.WasSuccessfullySensed()) return;
	//if (Stimulus.Tag != FName("Howling")) return;

	//// �̹� ���� ���̸� �Ҹ� ����
	//if (AnimalState == EAnimalState::Combat) return;

	//// ���� ������
	//float RandomDelay = FMath::RandRange(1.0f, 2.0f);
	//FTimerDelegate TimerDel;
	//TimerDel.BindWeakLambda(this, [this, Actor]()
	//	{
	//		// ������ �Ŀ��� Ÿ���� ��ȿ�ϰ� ���� ���� ���� �ƴ϶�� ���� ����
	//		if (IsValid(Actor) && EnemyState != EEnemyState::Combat && EnemyState != EEnemyState::Dead)
	//		{
	//			AttackTarget = Actor;
	//			SetEnemyState(EEnemyState::Combat);
	//		}
	//	});

	//GetWorld()->GetTimerManager().SetTimer(HearingReactTimer, TimerDel, RandomDelay, false);
}

//void AAnimalController::HandleScent(const FAIStimulus& Stimulus)
//{
//	//if (!Stimulus.WasSuccessfullySensed()) return;
//
//	//// ��ȭ�Ӱų� ���� ���� ���� ���� ����
//	//if (EnemyState == EEnemyState::Passive || EnemyState == EEnemyState::Investigating)
//	//{
//	//	// ���� ��ġ ����
//	//	if (UBlackboardComponent* BB = GetBlackboardComponent())
//	//	{
//	//		BB->SetValueAsVector(PointOfInterestKey, Stimulus.StimulusLocation);
//	//	}
//	//	SetEnemyState(EEnemyState::Investigating);
//	//}
//}