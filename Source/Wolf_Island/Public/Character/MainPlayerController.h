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

UCLASS()
class WOLF_ISLAND_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UInputMappingContext* InputMappingContext;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UInputAction* ChatAction;
	
	UPROPERTY(EditAnywhere)
	class AMainHUD* HUD;
	
	UPROPERTY(EditAnywhere)
	UChattingPanel* ChattingPanel;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UChattingPanel> ChattingPanelClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	AMainPlayerState* MainPlayerState;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	AMainGameState* MainGameState;
	
	UPROPERTY(BlueprintReadOnly)
	bool IsChat = false;
	
	virtual void BeginPlay() override;
	
	virtual void SetupInputComponent() override;
	
	UFUNCTION(BlueprintCallable)
	void ToggleChatMode();
	
	UFUNCTION()
	void EnterChatMode();
	UFUNCTION()
	void ExitChatMode();
	
	UFUNCTION(BlueprintCallable)
	void AddChat(FChattingData NewChattingData);
	
	//멀티 플레이 코드
	
	UFUNCTION(BlueprintCallable)
	void Request_SendChat(FChattingData NewChattingData);
	
	UFUNCTION(Server, Reliable)
	void Server_SendChat(FChattingData NewChattingData);
	
	UFUNCTION()
	void SendChat(FChattingData NewChattingData);
};