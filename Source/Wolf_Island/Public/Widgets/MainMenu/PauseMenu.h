// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenu.generated.h"

class UBaseButton;
class AMainPlayerController;
/**
 * 
 */

UCLASS()
class WOLF_ISLAND_API UPauseMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UBaseButton* ResumeButton;
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UBaseButton* SettingButton;

	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UBaseButton* QuitButton;
	
	UPROPERTY()
	AMainPlayerController* PlayerController;
	
	virtual void NativeConstruct() override;
	
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	
	UFUNCTION()
	void OnResumeClicked();
	UFUNCTION()
	void OnSettingClicked();
	UFUNCTION()
	void OnQuitClicked();
};

