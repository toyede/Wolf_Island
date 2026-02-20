// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputMappingContext.h"
#include "GameFramework/PlayerController.h"
#include "Games/MainGameState.h"
#include "Games/MainPlayerState.h"
#include "Widgets/Chatting/ChattingPanel.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */

class UMainGameInstance;
class UPauseMenu;

UCLASS()
class WOLF_ISLAND_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UInputMappingContext* InputMappingContext;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UInputAction* ChatAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UInputAction* ESCAction;
	
	UPROPERTY(EditAnywhere)
	class AMainHUD* HUD;
	
	UPROPERTY(EditAnywhere)
	UChattingPanel* ChattingPanel;
	
	UPROPERTY(EditAnywhere)
	UPauseMenu* PauseMenu;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UChattingPanel> ChattingPanelClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UPauseMenu> PauseWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UMainGameInstance* MainGameInstance;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	AMainPlayerState* MainPlayerState;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	AMainGameState* MainGameState;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftObjectPtr<UWorld> MainMenuLevel;
	
	UPROPERTY(BlueprintReadOnly)
	bool IsChat = false;
	
	UPROPERTY(BlueprintReadOnly)
	bool IsPause = false;
	
	virtual void BeginPlay() override;
	
	virtual void SetupInputComponent() override;
	
	UFUNCTION(BlueprintCallable)
	void ToggleChatMode();
	
	UFUNCTION(BlueprintCallable)
	void TogglePause();
	
	UFUNCTION()
	void EnterChatMode();
	UFUNCTION()
	void ExitChatMode();
	
	UFUNCTION()
	void DisplayPauseMenu();
	UFUNCTION()
	void HidePuaseMenu();
	
	UFUNCTION(BlueprintCallable)
	void AddChat(FChattingData NewChattingData);
	
	UFUNCTION(BlueprintCallable)
	void OnResume();
	UFUNCTION(BlueprintCallable)
	void OnSetting();
	UFUNCTION(BlueprintCallable)
	void OnQuit();
	
	//멀티 플레이 코드
	
	UFUNCTION(BlueprintCallable)
	void Request_SendChat(FChattingData NewChattingData);
	
	UFUNCTION(Server, Reliable)
	void Server_SendChat(FChattingData NewChattingData);
	
	UFUNCTION()
	void SendChat(FChattingData NewChattingData);
};