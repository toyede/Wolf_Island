// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Services/UBTService_UpdateTargetInfo.h"
#include "AI/Enemy_Character/EnemyAIBoss.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/MainPlayer.h"
#include "Components/StatusComponent.h"

UUBTService_UpdateTargetInfo::UUBTService_UpdateTargetInfo()
{
	NodeName = TEXT("Update Target Info");
	Interval = 0.5f;
	RandomDeviation = 0.1f;
}

void UUBTService_UpdateTargetInfo::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC) return;

	APawn* BossPawn = AIC->GetPawn();
	if (!BossPawn) return;

	AEnemyAIBoss* Boss = Cast<AEnemyAIBoss>(BossPawn);
	if (!Boss) return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	TArray<AMainPlayer*> ValidParticipants;
	for (AMainPlayer* Participant : Boss->BossParticipants)
	{
		if (!IsValid(Participant) || !IsValid(Participant->StatusComponent))
		{
			continue;
		}

		if (Participant->StatusComponent->IsDead || Participant->StatusComponent->CurrentHP <= 0.0f
			|| Participant->StatusComponent->bIsIncapacitated)
		{
			continue;
		}

		// 늑대인간으로 변신한 플레이어 제외
		if (Participant->ActorHasTag(FName("Werewolf"))) continue;

		ValidParticipants.Add(Participant);
	}

	if (ValidParticipants.Num() == 0)
	{
		BB->ClearValue(TargetKey.SelectedKeyName);
		BB->SetValueAsFloat(DistanceKey.SelectedKeyName, TNumericLimits<float>::Max());
		return;
	}

	AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));
	const bool bHasValidCurrentTarget = CurrentTarget && ValidParticipants.Contains(Cast<AMainPlayer>(CurrentTarget));

	AActor* SelectedTarget = CurrentTarget;
	if (!bHasValidCurrentTarget || bRetargetEveryTick)
	{
		switch (SelectionMode)
		{
		case EBossTargetSelectionMode::LowestHPParticipant:
			{
				AMainPlayer* LowestHPPlayer = nullptr;
				float LowestHP = TNumericLimits<float>::Max();

				for (AMainPlayer* Candidate : ValidParticipants)
				{
					const float CandidateHP = Candidate->StatusComponent->CurrentHP;
					if (!LowestHPPlayer || CandidateHP < LowestHP)
					{
						LowestHP = CandidateHP;
						LowestHPPlayer = Candidate;
					}
				}

				SelectedTarget = LowestHPPlayer;
			}
			break;

		case EBossTargetSelectionMode::RandomParticipant:
		default:
			{
				const int32 RandomIndex = FMath::RandRange(0, ValidParticipants.Num() - 1);
				SelectedTarget = ValidParticipants[RandomIndex];
			}
			break;
		}

		BB->SetValueAsObject(TargetKey.SelectedKeyName, SelectedTarget);
	}

	if (SelectedTarget)
	{
		const float Distance = FVector::Dist(BossPawn->GetActorLocation(), SelectedTarget->GetActorLocation());
		BB->SetValueAsFloat(DistanceKey.SelectedKeyName, Distance);
	}
}
