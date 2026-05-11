// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NativeSpawnActor.generated.h"

class AEnemyAIBase;
class APatrolRoute;

UCLASS()
class WOLF_ISLAND_API ANativeSpawnActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ANativeSpawnActor();

protected:
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	TSubclassOf<class AEnemyAIBase> NativeClass;

	// 기본 스폰 주기
	UPROPERTY(EditDefaultsOnly, Category = "Spawn")
	float SpawnInterval = 120.0f;
	
	UPROPERTY(EditInstanceOnly, Category = "Spawn")
	TObjectPtr<APatrolRoute> NativePatrolRoute;
	
	UPROPERTY(EditInstanceOnly, Category = "Spawn")
	TObjectPtr<APatrolRoute> WolfPatrolRoute;
	
	UPROPERTY(EditInstanceOnly, Category = "Spawn")
	int32 SpawnCountPerInterval = 5;
	
	UPROPERTY(EditInstanceOnly, Category = "Spawn")
	int32 MaxSpawnCount = 10;
	
	UPROPERTY(EditInstanceOnly, Category = "Spawn")
	float SpawnRandomDistance = 150.f;
	
	UPROPERTY()
	TArray<TWeakObjectPtr<AEnemyAIBase>> SpawnedNatives;
	
	FTimerHandle SpawnTimerHandle;
	
	void SpawnNative();
};
