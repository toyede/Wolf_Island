// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/InventorySlot.h"

#include "Components/InventoryComponent.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Item/ItemBase.h"
#include "Widgets/Inventory/DragItemVisual.h"
#include "Widgets/Inventory/InventoryToolTip.h"
#include "Widgets/Inventory/ItemDragDropOperation.h"

void UInventorySlot::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ToolTipClass && ItemRef)
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
			ItemAmount->SetVisibility(ESlateVisibility::Visible);
			ItemAmount->SetText(FText::AsNumber(ItemRef->Amount));
		} else
		{
			ItemAmount->SetVisibility(ESlateVisibility::Collapsed);
		}
	} else
	{
		ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
		ItemAmount->SetVisibility(ESlateVisibility::Collapsed);
	}
}

FReply UInventorySlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	//왼쪽 마우스 클릭이면
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return Reply.Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}

	return Reply.Unhandled();
}

void UInventorySlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
}

void UInventorySlot::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	
	if (DragItemVisualClass && ItemRef)
	{
		const TObjectPtr<UDragItemVisual> DragVisual = CreateWidget<UDragItemVisual>(this, DragItemVisualClass);
		DragVisual->ItemIcon->SetBrushFromTexture(ItemRef->AssetData.Icon);
		DragVisual->ItemBorder->SetBrushColor(ItemBorder->GetBrushColor());
		DragVisual->ItemAmount->SetText(FText::AsNumber(ItemRef->Amount));

		UItemDragDropOperation* DragItemOperation = NewObject<UItemDragDropOperation>();
		DragItemOperation->SourceItem = ItemRef;
		DragItemOperation->SourceInventory = ItemRef->OwningInventory;
		DragItemOperation->SourceIndex = Index;

		DragItemOperation->DefaultDragVisual = DragVisual;
		DragItemOperation->Pivot = EDragPivot::MouseDown;

		OutOperation = DragItemOperation;
		if (OutOperation)
		{
			UE_LOG(LogTemp, Warning, TEXT("DRAG ITEM OPERATION OUTTED"));
		}
	}
}

bool UInventorySlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	const UItemDragDropOperation* ItemDragDrop = Cast<UItemDragDropOperation>(InOperation);

	UE_LOG(LogTemp, Warning, TEXT("SLOT DROP DETECTED"));
	if (UInventoryComponent* InventoryRef = ItemDragDrop->SourceInventory)
	{
		InventoryRef->SwapItems(ItemDragDrop->SourceIndex, Index);
		return true;
	}
	
	return false;
}
