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
		if (ItemData)
		{
			ItemName->SetText(ItemData->TextData.Name);

			ItemDescription->SetText(ItemData->TextData.Description);

			ItemWeight->SetText(FText::AsNumber(ItemData->NumericData.Weight));
		}
		if (InventorySlotBeingHovered && ItemDurability)
		{
			const FItemBaseData SlotItem = InventorySlotBeingHovered->GetItemReference();

			const bool bHasDurability =
				(SlotItem.MaxDurability > 0.0f);

			if (bHasDurability)
			{
				const FText DurabilityText = FText::Format(
					FText::FromString(TEXT("내구도: {0} / {1}")),
					FText::AsNumber(FMath::FloorToInt(SlotItem.CurrentDurability)),
					FText::AsNumber(FMath::FloorToInt(SlotItem.MaxDurability))
				);

				ItemDurability->SetText(DurabilityText);
				ItemDurability->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				ItemDurability->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	} 
	else
	{
		ItemName->SetText(ItemData->TextData.Name);

		ItemDescription->SetText(ItemData->TextData.Description);

		ItemWeight->SetText(FText::AsNumber(ItemData->NumericData.Weight));
	}

	
}
