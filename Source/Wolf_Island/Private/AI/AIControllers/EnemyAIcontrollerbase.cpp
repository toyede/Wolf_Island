// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/AIControllers/EnemyAIcontrollerbase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Kismet/GameplayStatics.h"

AEnemyAIControllerBase::AEnemyAIControllerBase()
{
	BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
}

void AEnemyAIControllerBase::BeginPlay()
{
	Super::BeginPlay();

	if (AIPerceptionComp)
	{
		AIPerceptionComp->OnPerceptionUpdated.AddDynamic(this, &AEnemyAIControllerBase::OnPerceptionUpdated);
	}
}

void AEnemyAIControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AEnemyAIControllerBase::OnUnPossess()
{
	Super::OnUnPossess();

	if (BehaviorComp)
	{
		BehaviorComp->StopTree(EBTStopMode::Safe);
	}
}

void AEnemyAIControllerBase::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	// 자식 클래스에서 구현
}