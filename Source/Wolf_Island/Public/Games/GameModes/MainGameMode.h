// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Games/MainSaveGame.h"
#include "MainGameMode.generated.h"

class AMainPlayerState;
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
	TArray<FRemovedFoliageData> RemovedFoliageData;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TMap<FString, FPlayerSaveData> PlayersSaveData;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UMainSaveGame* CurrentSaveData;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UMainSaveGame* MorningSaveData;
	
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	UMainGameInstance* MainGameInstance;
	
	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<AMainPlayer>> PlayerRoleClassList;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FVector> SpawnPoints;
	
	//자동 저장 간격 (분 단위)
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AutoSaveInterval = 5.0f;
	
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	
	virtual void StartPlay() override;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	
	virtual void RestartPlayer(AController* NewPlayer) override;
	
	virtual void Logout(AController* Exiting) override;
	
	UFUNCTION(BlueprintCallable)
	virtual void StartingNewPlayer(APlayerController* NewPlayer);
	
	UFUNCTION(BlueprintCallable)
	void SetActorCache();
	
	UFUNCTION(BlueprintCallable)
	void SaveWorld();
	
	UFUNCTION(BlueprintCallable)
	void LoadWorld();
	
	UFUNCTION(BlueprintCallable)
	void SavePlayer(AMainPlayerState* PlayerState);
	
	UFUNCTION(BlueprintCallable)
	bool LoadPlayer(AMainPlayerState* PlayerState);
	
	UFUNCTION(BlueprintCallable)
	bool HasSaveData(FString PlayerId);
	
	UFUNCTION(BlueprintCallable)
	UMainSaveGame* DuplicateSaveData(UMainSaveGame* TargetSaveGame);
	
	//아침 시간에 이 함수를 작동시키면 아침 시점 데이터가 세이브 됨
	UFUNCTION(BlueprintCallable)
	void SaveMorningSaveData();
	
	//이건 플레이어가 죽었을 때 할 동작들. 여기선 싱글에서 죽었을 때를 구현하고, MultiGameMode에서 멀티에서 죽얼을 때 구현.
	UFUNCTION(BlueprintCallable)
	virtual void HandlePlayerDeath(AController* DeadPlayerController);
};
