#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ThrowObject.generated.h"

UCLASS()
class WOLF_ISLAND_API UBTTask_ThrowObject : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_ThrowObject();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    // 이 함수가 있어야 BT가 태스크를 중간에 취소할 수 있습니다.
    virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp);
};