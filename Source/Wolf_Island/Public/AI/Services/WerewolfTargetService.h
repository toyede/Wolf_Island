#include "BehaviorTree/BTService.h"
#include "WerewolfTargetService.generated.h"

UCLASS()
class WOLF_ISLAND_API UWerewolfTargetService : public UBTService
{
    GENERATED_BODY()

public:
    UWerewolfTargetService();

protected:
    // 서비스가 주기적으로 호출하는 핵심 함수입니다.
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    // 블랙보드 키를 에디터에서 선택할 수 있게 변수로 선언합니다.
    UPROPERTY(EditAnywhere, Category = "AI")
    FBlackboardKeySelector TargetActorKey;

    UPROPERTY(EditAnywhere, Category = "AI")
    float DetectionRange = 2000.f;
};