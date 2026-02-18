// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ExitButton.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UExitButton : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(meta=(BindWidget))
	UButton* ExitButton;
	
	UFUNCTION(BlueprintCallable)
	void OnExit();
	
	virtual void NativeConstruct() override;	
};
