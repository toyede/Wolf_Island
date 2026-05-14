// BTTask_PatrolRoute.cpp
#include "AI/Tasks/BTTask_patrolRoute.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Components/SplineComponent.h"
#include "AI/AIControllers/EnemyAIController.h"
#include "AI/Enemy_Character/EnemyAIBase.h"
#include "Actors/PatrolRoute.h"

UBTTask_PatrolRoute::UBTTask_PatrolRoute()
{
	NodeName = TEXT("Patrol Route");
	bNotifyTaskFinished = true;
	bCreateNodeInstance = true; // AI마다 이 태스크 UObject를 별도 생성 → 멤버 변수 공유 문제 없음
}

EBTNodeResult::Type UBTTask_PatrolRoute::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyAIController* AIC = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
	if (!AIC) return EBTNodeResult::Failed;

	TObjectPtr<AEnemyAIBase> Enemy = AIC->ControlledEnemy;
	if (!Enemy || !Enemy->AssignedPatrolRoute) return EBTNodeResult::Failed;

	USplineComponent* Spline = Enemy->AssignedPatrolRoute->SplinePoints;
	if (!Spline) return EBTNodeResult::Failed;

	// 포인트가 1개 이하면 순찰 의미 없음
	if (Spline->GetNumberOfSplinePoints() <= 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PatrolRoute] 스플라인 포인트가 1개 이하 - 순찰 불가"));
		return EBTNodeResult::Failed;
	}

	int32 NextIndex = Enemy->GetNextPoint();
	FVector TargetLocation = Spline->GetLocationAtSplinePoint(NextIndex, ESplineCoordinateSpace::World);

	// 이미 목적지에 너무 가까우면 (AlreadyAtGoal 무한루프 방지) 바로 성공 처리
	const float DistSq = FVector::DistSquared(Enemy->GetActorLocation(), TargetLocation);
	if (DistSq <= 50.f * 50.f)
	{
		return EBTNodeResult::Succeeded;
	}

	FAIMoveRequest MoveReq(TargetLocation);
	MoveReq.SetAcceptanceRadius(50.f);

	FNavPathSharedPtr NavPath;
	const FPathFollowingRequestResult MoveResult = AIC->MoveTo(MoveReq, &NavPath);

	if (MoveResult.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		return EBTNodeResult::Succeeded;
	}

	if (MoveResult.Code == EPathFollowingRequestResult::RequestSuccessful)
	{
		CachedOwnerComp = &OwnerComp;

		AIC->GetPathFollowingComponent()->OnRequestFinished.AddUObject(
			this, &UBTTask_PatrolRoute::OnMoveCompleted
		);

		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void UBTTask_PatrolRoute::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (CachedOwnerComp.IsValid())
	{
		FinishLatentTask(*CachedOwnerComp.Get(),
			Result.IsSuccess() ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
	}
}

void UBTTask_PatrolRoute::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	if (AAIController* AIC = OwnerComp.GetAIOwner())
	{
		if (UPathFollowingComponent* PFC = AIC->GetPathFollowingComponent())
		{
			PFC->OnRequestFinished.RemoveAll(this);
		}
	}

	CachedOwnerComp.Reset();
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}