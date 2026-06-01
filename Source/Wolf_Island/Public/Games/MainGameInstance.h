// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"
#include "MainGameInstance.generated.h"

class UMainSaveGame;
/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UMainGameInstance : public UAdvancedFriendsGameInstance
{
	GENERATED_BODY()
	
public:
	
	//로비 이름
	UPROPERTY(BlueprintReadWrite)
	FString CurrentServerName;
	
	UPROPERTY(BlueprintReadWrite)
	UMainSaveGame* CurrenSaveGame;
	
	UPROPERTY(BlueprintReadWrite)
	int32 MaxSlotIndex = 10;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftObjectPtr<UWorld> SinglePlayWorld;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftObjectPtr<UWorld> MultiPlayWorld;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftObjectPtr<UWorld> MultiLobbyWorld;
	
	virtual void Init() override;
	
	UFUNCTION(BlueprintCallable)
	UMainSaveGame* CreateSaveSlot(FString WorldName, int32 SlotIndex, bool IsMulti = false);
	
	//남는 슬롯 없으면 -1 반환
	UFUNCTION(BlueprintCallable)
	int32 FindEmptySaveSlotIndex(bool IsMulti);
	
	UFUNCTION(BlueprintCallable)
	int32 GetMaxSlotIndex() const { return MaxSlotIndex; }
	
	UFUNCTION(BlueprintCallable)
	void SetCurrentSave(UMainSaveGame* SaveGame) { CurrenSaveGame = SaveGame; }
	
	//세션 함수
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CreateSession();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CreateLobbySession();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void CreateGameSession();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void FindSession(const FString& SessionCode);

	//JWY-인게임/로비 Quit이 같은 세션 정리 흐름을 쓰도록 BP_MainGameInstance에서 DestroySession 후 메인 이동을 연결하기 위해 추가
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DestroySessionAndReturnToMainMenu();
	
	UFUNCTION(BlueprintCallable)
	FString GenerateSessionCode();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
	FString GetSessionCode();
};
