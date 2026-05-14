// BTTask_PatrolRoute.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Navigation/PathFollowingComponent.h"
#include "BTTask_PatrolRoute.generated.h"

UCLASS()
class WOLF_ISLAND_API UBTTask_PatrolRoute : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_PatrolRoute();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

private:
	void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);

	// bCreateNodeInstance = true 덕분에 AI마다 인스턴스가 별도 생성되어 멤버 변수 안전
	UPROPERTY()
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
};