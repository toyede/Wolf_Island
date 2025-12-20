// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Inventory/InventorySlot.h"
#include "HotbarSlot.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UHotbarSlot : public UInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(meta=(BindWidget))
	UTextBlock* SlotNumber;

public:

	UFUNCTION()
	void SetSlotNumber(int32 SlotIndex);
};
