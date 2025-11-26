// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EnemyAIBossController.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"

AEnemyAIBossController::AEnemyAIBossController()
{
    BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp")); // 생성자에서는 컴포넌트 부착만
}

void AEnemyAIBossController::BeginPlay()
{
    Super::BeginPlay();

    Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

    SetAttackTarget();
}

void AEnemyAIBossController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BehaviorTreeAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);

        if (UBlackboardComponent* BB = GetBlackboardComponent())
        {
            BB->SetValueAsEnum(BossStateKey, static_cast<uint8>(EBossState::Idle));
        }

        // Player 있으면 설정
        SetAttackTarget();
    }
}

void AEnemyAIBossController::SetNewState(EBossState NewState)
{
    BossState = NewState;

    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->SetValueAsEnum(BossStateKey, static_cast<uint8>(BossState));
    } 
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

    if (Candidates.Num() > 0)
    {
        int32 Index = FMath::RandRange(0, Candidates.Num() - 1);
        SetNewState(Candidates[Index]);
    }
}

EBossState AEnemyAIBossController::SetStateAsGroggy()
{
    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        if (BB->GetValueAsEnum(BossStateKey) == static_cast<uint8>(EBossState::Rush))
        {
            BB->SetValueAsEnum(BossStateKey, static_cast<uint8>(EBossState::Groggy));

            return EBossState::Groggy;
        }
    }
    return BossState;
}

EBossState AEnemyAIBossController::SetStateAsStun()
{
    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->SetValueAsEnum(BossStateKey, static_cast<uint8>(EBossState::Stun));

        return EBossState::Stun;
    }
    return BossState;
}

void AEnemyAIBossController::SetAttackTarget()
{
    // 둘 다 준비됐을 때만 설정
    if (Player)
    {
        if (UBlackboardComponent* BB = GetBlackboardComponent())
        {
            BB->SetValueAsObject(AttackTargetKey, Player);
        }
    }
}