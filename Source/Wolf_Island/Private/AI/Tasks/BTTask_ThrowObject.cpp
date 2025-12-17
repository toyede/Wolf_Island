#include "AI/Tasks/BTTask_ThrowObject.h"
#include "AIController.h"
#include "AI/Enemy_Character/EnemyAIBase.h"
#include "Animation/AnimInstance.h"
#include "AI/Interfaces/EnemyCommonInterface.h" 

UBTTask_ThrowObject::UBTTask_ThrowObject()
{
    NodeName = TEXT("Throw Object");
    bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_ThrowObject::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    AEnemyAIBase* Enemy = AICon ? Cast<AEnemyAIBase>(AICon->GetPawn()) : nullptr;

    if (!Enemy) return EBTNodeResult::Failed;

    // 캐싱
    CachedEnemy = Enemy;
    CachedOwnerComp = &OwnerComp;

    // 1. 이벤트 바인딩 (Enemy가 작업 끝났다고 할 때까지 대기)
    Enemy->OnThrowEnd.AddDynamic(this, &UBTTask_ThrowObject::OnThrowFinished);

    // 2. 실행 명령 (몽타주 재생은 Enemy 내부에서 알아서 함)
    if (Enemy->Implements<UEnemyCommonInterface>())
    {
        IEnemyCommonInterface::Execute_ThrowObject(Enemy);
        return EBTNodeResult::InProgress;
    }

    return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_ThrowObject::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // 태스크가 중단될 때 (예: 데미지 입어서 상태 변경됨)
    if (CachedEnemy.IsValid())
    {
        // 바인딩 해제 (중요: 해제 안 하면 태스크 끝나고도 호출될 수 있음)
        CachedEnemy->OnThrowEnd.RemoveDynamic(this, &UBTTask_ThrowObject::OnThrowFinished);
    }

    return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_ThrowObject::OnThrowFinished()
{
    // Enemy 쪽에서 호출해줌
    if (CachedEnemy.IsValid())
    {
        // 바인딩 해제
        CachedEnemy->OnThrowEnd.RemoveDynamic(this, &UBTTask_ThrowObject::OnThrowFinished);
    }

    if (CachedOwnerComp.IsValid())
    {
        // 태스크 성공 종료
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
    }
}
