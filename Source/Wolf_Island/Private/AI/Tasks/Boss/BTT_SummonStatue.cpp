// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/Boss/BTT_SummonStatue.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UBTT_SummonStatue::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (!StatueClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("StatueClass not set"));
		return EBTNodeResult::Failed;
	}

	AAIController* AICon = OwnerComp.GetAIOwner();
	APawn* AIPawn = AICon ? AICon->GetPawn() : nullptr;

	if (!AIPawn) return EBTNodeResult::Failed;

	UWorld* World = AIPawn->GetWorld();
	if (!World) return EBTNodeResult::Failed;

	FVector SpawnLocation = AIPawn->GetActorLocation() + AIPawn->GetActorForwardVector() * 400.f;
	FRotator SpawnRotation = AIPawn->GetActorRotation();

	AActor* Spawned = World->SpawnActor<AActor>(StatueClass, SpawnLocation, SpawnRotation);
	if (Spawned)
	{
		UE_LOG(LogTemp, Log, TEXT("Statue spawned"));
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
