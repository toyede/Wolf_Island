// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EnemyAIBossController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

AEnemyAIBossController::AEnemyAIBossController()
{
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
    BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));

    SetNewState(EBossState::Idle);
}

void AEnemyAIBossController::OnPossess(APawn* InPawn)
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

void AEnemyAIBossController::BeginPlay()
{
    Super::BeginPlay();

    AttackTarget = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    BlackboardComp->SetValueAsObject(AttackTargetKey, AttackTarget);
}

void AEnemyAIBossController::SetNewState(EBossState NewState)
{
    BossState = NewState;

    BlackboardComp->SetValueAsEnum(BossStateKey, static_cast<uint8>(BossState));
    
}

void AEnemyAIBossController::SetRandomNewState()
{
    // Not Include Groggy

    int32 RandomIndex = FMath::RandRange(0, static_cast<int32>(EBossState::Groggy) - 1);

    EBossState RandomState = static_cast<EBossState>(RandomIndex);

    BossState = RandomState;

    BlackboardComp->SetValueAsEnum(BossStateKey, static_cast<uint8>(BossState));

}