// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/NativeSpawnActor.h"
#include "AI/Enemy_Character/EnemyAIBase.h"
#include "Actors/PatrolRoute.h"

ANativeSpawnActor::ANativeSpawnActor()
{
	PrimaryActorTick.bCanEverTick = false;

}

void ANativeSpawnActor::BeginPlay()
{
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ANativeSpawnActor::SpawnNative, SpawnInterval, true, 0.f);
	
}

void ANativeSpawnActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void ANativeSpawnActor::SpawnNative()
{
	if (!HasAuthority() || !NativeClass) return;
	
	SpawnedNatives.RemoveAll([](const TWeakObjectPtr<AEnemyAIBase>& E)
	{
		return !E.IsValid();
	});
	
	if (SpawnedNatives.Num() >= MaxSpawnCount) return;
	
	int32 CanSpawn = MaxSpawnCount - SpawnedNatives.Num();
	int32 ToSpawn = FMath::Min(SpawnCountPerInterval, CanSpawn);
	
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (int32 i = 0; i < ToSpawn; i++)
	{
		FVector SpawnOffset = FVector(
			FMath::RandRange(-SpawnRandomDistance, SpawnRandomDistance),
			FMath::RandRange(-SpawnRandomDistance, SpawnRandomDistance),
			0.f);
		
		FVector SpawnLocation = GetActorLocation() + SpawnOffset;
		AEnemyAIBase* Enemy = GetWorld()->SpawnActor<AEnemyAIBase>(NativeClass, SpawnLocation, GetActorRotation(), Params);
	
		if (Enemy)
		{
			if (NativePatrolRoute) Enemy->NativePatrolRoute = NativePatrolRoute;
			if (WolfPatrolRoute) Enemy->WolfPatrolRoute = WolfPatrolRoute;
			SpawnedNatives.Add(Enemy);
		}
	}
}
