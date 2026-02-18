// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Games/MainSaveGame.h"
#include "MainGameMode.generated.h"

class UMainGameInstance;
/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API AMainGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	FTimerHandle AutoSaveTimer;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TMap<FGuid, AActor*> ActorCache;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TMap<FString, FPlayerSaveData> PlayersSaveData;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UMainSaveGame* SaveGameData;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UMainGameInstance* MainGameInstance;
	
	//자동 저장 간격 (분 단위)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AutoSaveInterval = 5.0f;
	
	virtual void StartPlay() override;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	
	virtual void Logout(AController* Exiting) override;
	
	UFUNCTION(BlueprintCallable)
	void SetActorCache();
	
	UFUNCTION(BlueprintCallable)
	void SaveWorld();
	
	UFUNCTION(BlueprintCallable)
	void LoadWorld();
	
	UFUNCTION(BlueprintCallable)
	void SavePlayer(class AMainPlayer* TargetPlayer);
	
	UFUNCTION(BlueprintCallable)
	bool LoadPlayer(AMainPlayer* TargetPlayer);
};
