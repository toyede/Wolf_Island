// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Games/MainSaveGame.h"
#include "MainGameMode.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API AMainGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	
	TMap<FGuid, AActor*> ActorCache;
	
	TMap<FString, FPlayerSaveData> PlayersSaveData;
	
	UMainSaveGame* SaveGameData;
	
	virtual void StartPlay() override;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	
	virtual void Logout(AController* Exiting) override;
	
	UFUNCTION(BlueprintCallable)
	void Save();
	
	UFUNCTION(BlueprintCallable)
	void Load();
	
	UFUNCTION(BlueprintCallable)
	void SavePlayer(class AMainPlayer* TargetPlayer);
	
	UFUNCTION(BlueprintCallable)
	bool LoadPlayer(AMainPlayer* TargetPlayer);
};
