// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/InventoryComponent.h"
#include "PlayerHUD.generated.h"

/**
 * 
 */

class UProgressBar;

UCLASS()
class WOLF_ISLAND_API UPlayerHUD : public UUserWidget
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
	void AddItemMessage(FItemAddResult Result);
		
	UPROPERTY(meta=(BindWidget))
	UProgressBar* InteractionBar;
	
	
};
