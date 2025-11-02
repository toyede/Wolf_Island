// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryToolTip.generated.h"

class UInventorySlot;
class UTextBlock;
/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UInventoryToolTip : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere)
	UInventorySlot* InventorySlotBeingHovered;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ItemName;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ItemDescription;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ItemWeight;

protected:

	virtual void NativeConstruct() override;
	
};
