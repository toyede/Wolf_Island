// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerCard.generated.h"

class UIconButton;
class UButton;
class AMainPlayerState;
class UTextBlock;
/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UPlayerCard : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UTextBlock* PlayerName;
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UTextBlock* PlayerReady;
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UIconButton* KickButton;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	APlayerController* PlayerController;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateColor DefaultNicknameColor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateColor ReadyNicknameColor;
	
	UFUNCTION(BlueprintCallable)
	void SetReady(bool IsReady);

	UFUNCTION(BlueprintCallable)
	void SetPlayerController(APlayerController* NewPlayerController) { PlayerController = NewPlayerController; }
	
	UFUNCTION(BlueprintCallable)
	void UpdateCard(AMainPlayerState* PlayerState);
	
	UFUNCTION(BlueprintCallable)
	void OnKickButtonClicked();
	
	UFUNCTION(BlueprintCallable)
	void SetKickButton(bool IsVisible);
	
	virtual void NativeConstruct() override;
};
