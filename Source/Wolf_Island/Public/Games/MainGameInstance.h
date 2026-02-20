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
		
	UPROPERTY(BlueprintReadWrite)
	UMainSaveGame* CurrenSaveGame;
	
	UPROPERTY(BlueprintReadWrite)
	int32 MaxSlotIndex = 5;
	
	UPROPERTY(BlueprintReadWrite)
	TSoftObjectPtr<UWorld> SinglePlayWorld;
	
	UPROPERTY(BlueprintReadWrite)
	TSoftObjectPtr<UWorld> MultiPlayWorld;

	UPROPERTY(BlueprintReadWrite)
	TSoftObjectPtr<UWorld> MultiLobbyWorld;
	
	UFUNCTION(BlueprintCallable)
	UMainSaveGame* CreateSaveSlot(FString WorldName, int32 SlotIndex, bool IsMulti = false);
	
	//남는 슬롯 없으면 -1 반환
	UFUNCTION(BlueprintCallable)
	int32 FindEmptySaveSlotIndex(bool IsMulti);
	
	UFUNCTION(BlueprintCallable)
	int32 GetMaxSlotIndex() const { return MaxSlotIndex; }
	
	UFUNCTION(BlueprintCallable)
	void SetCurrentSave(UMainSaveGame* SaveGame) { CurrenSaveGame = SaveGame; }
	
	virtual void Init() override;
};
