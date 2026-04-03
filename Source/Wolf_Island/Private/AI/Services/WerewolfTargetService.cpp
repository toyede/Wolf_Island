#include "AI/Services/WerewolfTargetService.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Character/MainPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StatusComponent.h"

UWerewolfTargetService::UWerewolfTargetService()
{
    NodeName = TEXT("Find Closest Player");
    // 실행 간격을 조절합니다 (예: 0.5초마다 체크)
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
        if (!MP || MP->IsHidden()) continue; // 변신한 본체 제외

        // 기절/사망자 제외
        if (MP->StatusComponent && MP->StatusComponent->bIsIncapacitated) continue;

        float Dist = FVector::Dist(ControllingPawn->GetActorLocation(), MP->GetActorLocation());
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestPlayer = MP;
        }
    }

    // 찾은 타겟을 블랙보드에 저장합니다.
    OwnerComp.GetBlackboardComponent()->SetValueAsObject(TargetActorKey.SelectedKeyName, ClosestPlayer);
}