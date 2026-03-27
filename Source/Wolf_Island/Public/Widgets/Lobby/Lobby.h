// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Lobby.generated.h"

class UBaseButton;
class ALobbyPlayerController;
class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API ULobby : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UDataTable* RoleDataTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ALobbyPlayerController* PlayerController;
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UButton* PrevButton;
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UButton* NextButton;
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UBaseButton* ReadyButton;
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UBaseButton* StartButton;
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UTextBlock* RoleName;
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UTextBlock* RoleDescription;
	
	UFUNCTION(BlueprintCallable)
	void OnPrev();
	UFUNCTION(BlueprintCallable)
	void OnNext();
	UFUNCTION(BlueprintCallable)
	void OnReady();
	
	UFUNCTION(BlueprintCallable)
	void RefreshInfo();
	
	UFUNCTION(BlueprintCallable)
	void SwitchPlayButton(bool On);
	
	UFUNCTION(BlueprintCallable)
	void SwitchRoleButton(bool IsOn);
	
	virtual void NativeConstruct() override;
};
