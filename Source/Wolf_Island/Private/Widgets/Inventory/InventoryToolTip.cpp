// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/InventoryToolTip.h"
#include "Widgets/Inventory/InventorySlot.h"

#include "Components/TextBlock.h"
#include "Item/ItemBase.h"

void UInventoryToolTip::NativeConstruct()
{
	Super::NativeConstruct();
	
	

	if (InventorySlotBeingHovered)
	{
		UItemBase* Item = InventorySlotBeingHovered->GetItemReference();
		
		if (Item)
		{
			ItemName->SetText(Item->TextData.Name);

			ItemDescription->SetText(Item->TextData.Description);

			ItemWeight->SetText(FText::AsNumber(Item->NumericData.Weight));
		}
	} 
	else
	{
		ItemName->SetText(ItemData.TextData.Name);

		ItemDescription->SetText(ItemData.TextData.Description);

		ItemWeight->SetText(FText::AsNumber(ItemData.NumericData.Weight));
	}
}
