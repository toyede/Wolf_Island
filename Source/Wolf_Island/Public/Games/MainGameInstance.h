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

	// JWY - C++ Quit 흐름에서 BP_MainGameInstance의 DestroySession 노드를 거친 뒤 메인 메뉴로 돌아갈 수 있도록 공통 진입점을 제공합니다.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void DestroySessionAndReturnToMainMenu(const TSoftObjectPtr<UWorld>& TargetMainMenuLevel);
	
	UFUNCTION(BlueprintCallable)
	FString GenerateSessionCode();
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, BlueprintPure)
	FString GetSessionCode();
};
