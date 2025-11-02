// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Inventory.h"

#include "Character/MainPlayer.h"
#include "Components/InventoryComponent.h"
#include "Components/SizeBox.h"
#include "Item/ItemBase.h"
#include "Widgets/Inventory/InventoryPanel.h"
#include "Widgets/Inventory/ItemDragDropOperation.h"

void UInventory::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UInventory::NativeConstruct()
{
	Super::NativeConstruct();

	PlayerRef = Cast<AMainPlayer>(GetOwningPlayerPawn());
}

bool UInventory::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	const UItemDragDropOperation* ItemDragDrop = Cast<UItemDragDropOperation>(InOperation);

	UItemBase* Item = ItemDragDrop->SourceItem;
	
	if (PlayerRef && Item)
	{
		if (InventoryPanel)
		{
			FVector2D LocalMousePos = InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
			
			FGeometry PanelGeometry = InventorySection->GetCachedGeometry();
			FVector2D PanelPos = InGeometry.AbsoluteToLocal(PanelGeometry.GetAbsolutePosition());
			FVector2D PanelSize = PanelGeometry.GetLocalSize();

			// 마우스가 패널 영역 안에 있다면 드랍 무시
			if (LocalMousePos.X >= PanelPos.X && LocalMousePos.X <= PanelPos.X + PanelSize.X &&
				LocalMousePos.Y >= PanelPos.Y && LocalMousePos.Y <= PanelPos.Y + PanelSize.Y)
			{
				//우클릭이면 반갈한 거 원위치
				if (InDragDropEvent.GetEffectingButton() == EKeys::RightMouseButton)
				{
					ItemDragDrop->SourceInventory->GetItemAtIndex(ItemDragDrop->SourceIndex)->Amount += ItemDragDrop->SourceItem->Amount;
					ItemDragDrop->SourceInventory->OnInventoryUpdated.Broadcast();
				}
				return false;
			}
		}
		//우클릭이면 반갈한 것만 버리기
		if (InDragDropEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			PlayerRef->DropItem(Item, Item->Amount, false);
			return true;
		}
		
		PlayerRef->DropItem(Item, Item->Amount, true);
		return true;
	}
	return false;
}

