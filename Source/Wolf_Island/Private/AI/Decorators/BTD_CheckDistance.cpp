// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Decorators/BTD_CheckDistance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Actor.h"

UBTD_CheckDistance::UBTD_CheckDistance()
{
	MaxDistance = 500.f; // 디폴트값
}

bool UBTD_CheckDistance::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return false;

	// Player Actor 가져오기
	AActor* PlayerActor = Cast<AActor>(BlackboardComp->GetValueAsObject(PlayerKey.SelectedKeyName));
	// Boss = 이 데코레이터를 실행하는 AI의 Pawn
	APawn* AIPawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;

	if (!PlayerActor || !AIPawn) return false;

	// 거리 계산
	float Distance = FVector::Dist(PlayerActor->GetActorLocation(), AIPawn->GetActorLocation());

	return Distance <= MaxDistance;
}
