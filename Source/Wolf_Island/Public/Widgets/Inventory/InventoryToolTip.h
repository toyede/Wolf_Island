// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemDataStruct.h"
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

	//UPROPERTY(VisibleAnywhere)
	FItemData* ItemData;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ItemName;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ItemDescription;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ItemWeight;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ItemDurability;

protected:

	virtual void NativeConstruct() override;
	
};
