// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Services/UBTService_UpdateTargetInfo.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/MainPlayer.h"
#include "Kismet/GameplayStatics.h"

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

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));

	if (!CurrentTarget)
	{
		TArray<AActor*> Players;
		UGameplayStatics::GetAllActorsOfClass(BossPawn->GetWorld(), AMainPlayer::StaticClass(), Players);
	
		if (Players.Num() > 0)
		{
			int32 RandomIndex = FMath::RandRange(0, Players.Num() - 1);

			CurrentTarget = Players[RandomIndex];
			BB->SetValueAsObject(TargetKey.SelectedKeyName, CurrentTarget);

			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("New Target: %s"), *CurrentTarget->GetName()));
		}
	}

	if (CurrentTarget)
	{
		float Distance = FVector::Dist(BossPawn->GetActorLocation(), CurrentTarget->GetActorLocation());
		BB->SetValueAsFloat(DistanceKey.SelectedKeyName, Distance);
	}
}
