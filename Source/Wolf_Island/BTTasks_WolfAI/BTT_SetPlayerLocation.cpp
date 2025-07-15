#include "BTT_SetPlayerLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"

UBTT_SetPlayerLocation::UBTT_SetPlayerLocation()
{
    NodeName = TEXT("Set Player Location");
    // 키는 반드시 블랙보드 상에 Vector 타입으로 미리 만들어 두세요.
}

EBTNodeResult::Type UBTT_SetPlayerLocation::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory
) {
    // AIController 가져오기
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC) return EBTNodeResult::Failed;

    // 플레이어 Pawn 가져오기 (인덱스 0)
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(AIC, 0);
    if (!PlayerPawn) return EBTNodeResult::Failed;

    // 위치 저장
    const FVector PlayerLoc = PlayerPawn->GetActorLocation();
    OwnerComp.GetBlackboardComponent()
        ->SetValueAsVector(PlayerLocationKey.SelectedKeyName, PlayerLoc);

    return EBTNodeResult::Succeeded;
}
