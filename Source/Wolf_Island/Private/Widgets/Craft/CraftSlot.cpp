// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Craft/CraftSlot.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/ItemDataStruct.h"
#include "Widgets/Inventory/InventoryToolTip.h"

void UCraftSlot::NativeConstruct()
{
	Super::NativeConstruct();
}

void UCraftSlot::SetCraftSlot(FItemData* ItemData, int32 Amount)
{
	if (ItemData->AssetData.Icon)
	{
		ItemIcon->SetVisibility(ESlateVisibility::Visible);
		ItemIcon->SetBrushFromTexture(ItemData->AssetData.Icon);
	} else
	{
		ItemIcon->SetVisibility(ESlateVisibility::Hidden);
	}
	AmountText->SetText(Amount == 0 ? FText() : FText::AsNumber(Amount));

	if (ToolTipClass)
	{
		UInventoryToolTip* ToolTip = CreateWidget<UInventoryToolTip>(this, ToolTipClass);
		ToolTip->ItemData = ItemData;
		SetToolTip(ToolTip);
	}
}