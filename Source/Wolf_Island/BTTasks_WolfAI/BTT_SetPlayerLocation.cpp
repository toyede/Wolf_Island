#include "BTT_SetPlayerLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"

UBTT_SetPlayerLocation::UBTT_SetPlayerLocation()
{
    NodeName = TEXT("Set Player Location");
    // Ű�� �ݵ�� ������ �� Vector Ÿ������ �̸� ����� �μ���.
}

EBTNodeResult::Type UBTT_SetPlayerLocation::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory
) {
    // AIController ��������
    AAIController* AIC = OwnerComp.GetAIOwner();
    if (!AIC) return EBTNodeResult::Failed;

    // �÷��̾� Pawn �������� (�ε��� 0)
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(AIC, 0);
    if (!PlayerPawn) return EBTNodeResult::Failed;

    // ��ġ ����
    const FVector PlayerLoc = PlayerPawn->GetActorLocation();
    OwnerComp.GetBlackboardComponent()
        ->SetValueAsVector(PlayerLocationKey.SelectedKeyName, PlayerLoc);

    return EBTNodeResult::Succeeded;
}
