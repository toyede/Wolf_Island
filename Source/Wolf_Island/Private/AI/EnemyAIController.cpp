// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

AEnemyAIController::AEnemyAIController()
{
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
    BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));

    AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));

    // Set State As Passive
    EnemyState = EEnemyState::Passive;

    // Sight 
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 700.f;
    SightConfig->LoseSightRadius = 800.f;
    SightConfig->PeripheralVisionAngleDegrees = 60.f;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->SetMaxAge(1.5f);

    // Hearing 
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 800.f;

    // Perception Component
    AIPerceptionComp->ConfigureSense(*SightConfig);
    AIPerceptionComp->ConfigureSense(*HearingConfig);
    AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

    AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetPerceptionUpdated);
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BehaviorTreeAsset)
    {
        if (UseBlackboard(BehaviorTreeAsset->BlackboardAsset, BlackboardComp))
        {
            RunBehaviorTree(BehaviorTreeAsset);
        }
    }
}

void AEnemyAIController::BeginPlay()
{
    Super::BeginPlay();
}

void AEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

    if (Stimulus.WasSuccessfullySensed() && Actor == PlayerPawn)
    {
        if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Sight Sensed"));
            SetStateAsAttacking(PlayerPawn);
        }
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Sense Lost"));
        SetStateAsPassive();
        BlackboardComp->SetValueAsObject(AttackTargetKey, nullptr);
    }
}

void AEnemyAIController::SetStateAsPassive()
{
    EnemyState = EEnemyState::Passive;
    BlackboardComp->SetValueAsEnum(EnemyStateKey, static_cast<uint8>(EnemyState));
}

void AEnemyAIController::SetStateAsAttacking(AActor* Actor)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    EnemyState = EEnemyState::Attacking;
    BlackboardComp->SetValueAsEnum(EnemyStateKey, static_cast<uint8>(EnemyState));
    if (PlayerPawn != nullptr)
    {
        BlackboardComp->SetValueAsObject(AttackTargetKey, PlayerPawn);
    }
}
