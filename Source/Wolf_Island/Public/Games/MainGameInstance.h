// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"
#include "MainGameInstance.generated.h"

class UMainSaveGame;
class SWidget;
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

	virtual void Shutdown() override;

	//JWY-로비->게임 Seamless Travel 동안 Async Loading Screen의 Default Loading Screen과 같은 디자인의 로딩 화면을 띄울지 여부
	UPROPERTY(EditDefaultsOnly, Category = "Loading Screen")
	bool bShowSeamlessTravelLoadingScreen = true;

	//JWY-이동이 끝나지 않는 예외 상황에서 로딩 화면이 남아 있지 않도록 강제로 내리는 시간(초)
	UPROPERTY(EditDefaultsOnly, Category = "Loading Screen", meta = (ClampMin = "1.0"))
	float SeamlessTravelLoadingTimeout = 60.0f;

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

private:
	void HandleSeamlessTravelStart(UWorld* CurrentWorld, const FString& MapName);
	void HandlePostLoadMapWithWorld(UWorld* LoadedWorld);
	void ShowSeamlessTravelLoadingScreen();
	void RequestHideSeamlessTravelLoadingScreen();
	void HideSeamlessTravelLoadingScreen();

	TSharedPtr<SWidget> SeamlessTravelLoadingWidget;
	double SeamlessTravelLoadingShownTime = 0.0;
	FTimerHandle SeamlessTravelLoadingHideTimerHandle;
	FTimerHandle SeamlessTravelLoadingTimeoutHandle;

	FDelegateHandle SeamlessTravelStartHandle;
	FDelegateHandle PostLoadMapHandle;
	FDelegateHandle TravelFailureHandle;
	FDelegateHandle NetworkFailureHandle;
};
