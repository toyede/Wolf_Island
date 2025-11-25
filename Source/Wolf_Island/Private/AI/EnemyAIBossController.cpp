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
    TArray<EBossState> Candidates;
    Candidates.Add(EBossState::Idle);
    Candidates.Add(EBossState::Move);
    Candidates.Add(EBossState::ThrowAttack);
	Candidates.Add(EBossState::Rush);
	Candidates.Add(EBossState::SummonStatue);
	Candidates.Add(EBossState::SummonAltar);
	Candidates.Add(EBossState::Attack);
    // Groggy, Dead, Stun은 추가 안함

    int32 Index = FMath::RandRange(0, Candidates.Num() - 1);

    BossState = Candidates[Index];
    BlackboardComp->SetValueAsEnum(BossStateKey, (uint8)BossState);

}