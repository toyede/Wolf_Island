#include "AI/Services/WerewolfTargetService.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Character/MainPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StatusComponent.h"

UWerewolfTargetService::UWerewolfTargetService()
{
    NodeName = TEXT("Find Closest Player");
    // ���� ������ �����մϴ� (��: 0.5�ʸ��� üũ)
    Interval = 0.5f;
    RandomDeviation = 0.1f;
}

void UWerewolfTargetService::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
    if (!ControllingPawn) return;

    float ClosestDist = DetectionRange;
    AActor* ClosestPlayer = nullptr;

    TArray<AActor*> FoundPlayers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMainPlayer::StaticClass(), FoundPlayers);

    for (AActor* Actor : FoundPlayers)
    {
        AMainPlayer* MP = Cast<AMainPlayer>(Actor);
        if (!MP || MP->IsHidden()) continue; // ������ ��ü ����

        // 기절/쓰러진 상태 제외
        if (MP->IsInability) continue;

        float Dist = FVector::Dist(ControllingPawn->GetActorLocation(), MP->GetActorLocation());
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestPlayer = MP;
        }
    }

    // ã�� Ÿ���� �������忡 �����մϴ�.
    OwnerComp.GetBlackboardComponent()->SetValueAsObject(TargetActorKey.SelectedKeyName, ClosestPlayer);
}