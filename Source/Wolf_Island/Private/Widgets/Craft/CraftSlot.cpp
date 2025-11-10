// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Craft/CraftSlot.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/ItemDataStruct.h"

void UCraftSlot::SetCraftSlot(const FItemData& ItemData, int32 Amount)
{
	if (ItemData.AssetData.Icon)
	{
		ItemIcon->SetVisibility(ESlateVisibility::Visible);
		ItemIcon->SetBrushFromTexture(ItemData.AssetData.Icon);
	} else
	{
		ItemIcon->SetVisibility(ESlateVisibility::Hidden);
	}
	AmountText->SetText(Amount == 0 ? FText() : FText::AsNumber(Amount));
}
