// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "AI/EnemyAIBase.h"

AEnemyAIController::AEnemyAIController()
{
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
    BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
    AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));


    // Sight 
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1000.f;
    SightConfig->LoseSightRadius = 1700.f;
    SightConfig->PeripheralVisionAngleDegrees = 70.f;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->SetMaxAge(5.0f);

    // Hearing 
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 2000.f;
    HearingConfig->LoSHearingRange = 2500.f;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->SetMaxAge(2.0f);

    // Perception Component
    AIPerceptionComp->ConfigureSense(*SightConfig);
    AIPerceptionComp->ConfigureSense(*HearingConfig);
    AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());

    AIPerceptionComp->OnPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnPerceptionUpdated);
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    ControlledEnemy = Cast<AEnemyAIBase>(InPawn);

    if (BehaviorTreeAsset)
    {
        if (UseBlackboard(BehaviorTreeAsset->BlackboardAsset, BlackboardComp))
        {
            RunBehaviorTree(BehaviorTreeAsset);
            SetStateAsPassive();
        }
    }

    // Form Setting
    ControlledEnemy->ChangeForm(ControlledEnemy->EnemyForm);

    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AEnemyAIController::CheckIfForgottenSeenActor, 0.5f, true);
}

void AEnemyAIController::OnUnPossess()
{
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
    TimerHandle.Invalidate();
}

void AEnemyAIController::BeginPlay()
{
    Super::BeginPlay();
}

void AEnemyAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    for (AActor * Actor : UpdatedActors)
    {
        FAIStimulus Stimulus;
        if (CanSensedActor(Actor, Stimulus))
        {
            if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>() && Actor == PlayerPawn)
            {
                UE_LOG(LogTemp, Display, TEXT("percepted"));
                KnownSeenActors.AddUnique(Actor);
                SetStateAsAttacking(Actor);
            }
            else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>() && Stimulus.Tag == FName("Howling"))
            {
                float RandomDelay = FMath::RandRange(1.0f, 2.0f);

                GetWorld()->GetTimerManager().SetTimer(
                    HearingReactTimer,
                    FTimerDelegate::CreateLambda([this, PlayerPawn]()
                        {
                            SetStateAsAttacking(PlayerPawn);
                            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Reacted to Howl!"));
                        }),
                    RandomDelay,
                    false
                );
            }
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Red, TEXT("zz"));
        }
    }
}

bool AEnemyAIController::CanSensedActor(AActor* Actor, FAIStimulus& OutStimulus)
{
    if (!Actor || !AIPerceptionComp) return false;

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
    BlackboardComp->SetValueAsEnum(EnemyStateKey, static_cast<uint8>(EnemyState));
    BlackboardComp->SetValueAsObject(AttackTargetKey, nullptr);
}

void AEnemyAIController::SetStateAsAttacking(AActor* Actor)
{
    AttackTarget = Actor;
    EnemyState = EEnemyState::Attacking;
    BlackboardComp->SetValueAsEnum(EnemyStateKey, static_cast<uint8>(EnemyState));
    
    if (Actor != nullptr)
    {
        BlackboardComp->SetValueAsObject(AttackTargetKey, Actor);
    }
    else
    {
        SetStateAsPassive();
    }
}

void AEnemyAIController::CheckIfForgottenSeenActor()
{
    TArray<AActor*> KnownPerceived;
    AIPerceptionComp->GetKnownPerceivedActors(UAISense_Sight::StaticClass(), KnownPerceived);

    if (KnownSeenActors.Num() != KnownPerceived.Num())
    {
        for (AActor* Actor : KnownSeenActors)
        {
            int32 Index = KnownPerceived.Find(Actor);
            if (Index == INDEX_NONE)
            {
                HandleForgotActor(Actor);
            }
        }
    }
}
