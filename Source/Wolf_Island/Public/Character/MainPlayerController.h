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

class URoleSelection;
class UMainGameInstance;
class AMainGameMode;
class UPauseMenu;
class UPlayerHUD;

UCLASS()
class WOLF_ISLAND_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputMappingContext* InputMappingContext;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* ChatAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* ESCAction;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category="Widget")
	class AMainHUD* HUD;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category="Widget")
	UPlayerHUD* PlayerHUD;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category="Widget")
	UChattingPanel* ChattingPanel;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category="Widget")
	UPauseMenu* PauseMenu;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category="Widget")
	URoleSelection* RoleSelectionWidget;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UPlayerHUD> HUDClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Widget")
	TSubclassOf<UChattingPanel> ChattingPanelClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Widget")
	TSubclassOf<UPauseMenu> PauseWidgetClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Widget")
	TSubclassOf<URoleSelection> RoleSelectionWidgetClass;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UMainGameInstance* MainGameInstance;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AMainGameMode* MainGameMode;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AMainPlayerState* MainPlayerState;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AMainGameState* MainGameState;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftObjectPtr<UWorld> MainMenuLevel;
	
	UPROPERTY(BlueprintReadOnly)
	bool IsChat = false;
	
	UPROPERTY(BlueprintReadOnly)
	bool IsPause = false;
	
	virtual void BeginPlay() override;
	
	virtual void SetupInputComponent() override;
	
	virtual void OnPossess(APawn* InPawn) override;
	
	virtual void OnUnPossess() override;
	
	UFUNCTION(BlueprintCallable)
	void SetPlayerHUD(AMainPlayer* OwnerPlayer);
	
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
	void HidePauseMenu();
	
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
	
	//역할 선택 관련
	UFUNCTION(Client, Reliable)
	void Client_OpenSelectionUI();
	
	UFUNCTION(Server, Reliable)
	void Server_ConfirmRole(ECharacterRole NewRole);
	
	UFUNCTION(Client, Reliable)
	void Client_SetInputModeGame();
	
	UFUNCTION(Client, Reliable)
	void Client_RoleDeny();
	
	UFUNCTION(Client, Reliable)
	void Client_EndSelection();
	
	virtual void OnRep_PlayerState() override;
};