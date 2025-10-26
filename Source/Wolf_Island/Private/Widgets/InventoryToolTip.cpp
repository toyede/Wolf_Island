// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InventoryToolTip.h"

#include "Components/TextBlock.h"
#include "Item/ItemBase.h"
#include "Widgets/InventorySlot.h"

void UInventoryToolTip::NativeConstruct()
{
	Super::NativeConstruct();

	UItemBase* Item = InventorySlotBeingHovered->GetItemReference();
	
	ItemName->SetText(Item->TextData.Name);

	ItemDescription->SetText(Item->TextData.Description);

	ItemWeight->SetText(FText::AsNumber(Item->NumericData.Weight));
	
}
