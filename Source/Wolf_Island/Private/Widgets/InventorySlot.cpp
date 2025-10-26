// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/InventorySlot.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Item/ItemBase.h"
#include "Widgets/InventoryToolTip.h"

void UInventorySlot::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ToolTipClass)
	{
		UInventoryToolTip* ToolTip = CreateWidget<UInventoryToolTip>(this, ToolTipClass);
		ToolTip->InventorySlotBeingHovered = this;
		SetToolTip(ToolTip);
	}
}


void UInventorySlot::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemRef)
	{
		ItemIcon->SetBrushFromTexture(ItemRef->AssetData.Icon);

		if (ItemRef->NumericData.IsStackable)
		{
			ItemAmount->SetText(FText::AsNumber(ItemRef->Amount));
		} else
		{
			ItemAmount->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

FReply UInventorySlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UInventorySlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
}

void UInventorySlot::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
}

bool UInventorySlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}
