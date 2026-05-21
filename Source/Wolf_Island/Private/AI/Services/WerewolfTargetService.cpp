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

    // [Refactor] Tick 안전성: GetAIOwner() 및 GetPawn() null 체크
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Refactor] UWerewolfTargetService::TickNode: AIOwner is invalid"));
        return;
    }

    APawn* ControllingPawn = AICon->GetPawn();
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

    // [Refactor] Tick 안전성: BlackboardComponent null 체크 후 값 설정
    if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
    {
        BB->SetValueAsObject(TargetActorKey.SelectedKeyName, ClosestPlayer);
    }
}