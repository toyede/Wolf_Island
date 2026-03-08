// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Games/GameModes/MainGameMode.h"
#include "MultiGameMode.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API AMultiGameMode : public AMainGameMode
{
	GENERATED_BODY()
	
public:
	
	bool IsMulti = true;
	
	AMultiGameMode();
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	
	virtual void RestartPlayer(AController* NewPlayer) override;
	
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;
	
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
	
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	
	virtual void Logout(AController* Exiting) override;
	
	//멀티에서 플레이어 죽었을 때 동작 구현.
	virtual void HandlePlayerDeath(AController* DeadPlayerController) override;
};
