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
        if (!MP || MP->IsHidden()) continue; // 숨겨진 액터 제외

        // 기절/쓰러진 상태 제외
        if (MP->IsInability) continue;

        // 늑대인간으로 변신 중인 플레이어 제외 (Werewolf 태그)
        if (MP->ActorHasTag(FName("Werewolf"))) continue;

        // StatusComponent 기반 추가 방어 체크 (인캐퍼시테이트 상태 제외)
        if (UStatusComponent* SC = MP->FindComponentByClass<UStatusComponent>())
        {
            if (SC->bIsIncapacitated) continue;
        }

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