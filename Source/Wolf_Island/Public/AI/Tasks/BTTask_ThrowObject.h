#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ThrowObject.generated.h"

class AEnemyAIBase;

UCLASS()
class WOLF_ISLAND_API UBTTask_ThrowObject : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_ThrowObject();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
    UFUNCTION()
    void OnThrowFinished();

    // 안전한 언바인딩과 FinishLatentTask를 위해 캐싱
    TWeakObjectPtr<AEnemyAIBase> CachedEnemy;
    TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
};