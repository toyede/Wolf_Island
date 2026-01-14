// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Inventory.h"

#include "Character/MainPlayer.h"
#include "Components/Button.h"
#include "Components/InventoryComponent.h"
#include "Components/SizeBox.h"
#include "Components/WidgetSwitcher.h"
#include "Item/ItemBase.h"
#include "Widgets/Craft/CraftPanel.h"
#include "Widgets/Inventory/InventoryPanel.h"
#include "Widgets/Inventory/ItemDragDropOperation.h"

void UInventory::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	PanelSwitcher->AddChild(InventorySection);
	PanelSwitcher->AddChild(FoodRecipeSection);
	PanelSwitcher->AddChild(CraftRecipeSection);
	PanelSwitcher->AddChild(UnknownRecordSection);

	InventoryButton->OnClicked.AddDynamic(this, &UInventory::HandleInventoryClicked);
	FoodRecipeButton->OnClicked.AddDynamic(this, &UInventory::HandleFoodRecipeClicked);
	CraftRecipeButton->OnClicked.AddDynamic(this, &UInventory::HandleCraftRecipeClicked);
	UnknownRecordButton->OnClicked.AddDynamic(this, &UInventory::HandleUnknownRecordClicked);

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

	const FItemBaseData* ItemData = &ItemDragDrop->SourceItemData;
	
	if (PlayerRef && ItemData)
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
				//우클릭이면 떨구기면 반갈한 거 원위치
				if (InDragDropEvent.GetEffectingButton() == EKeys::RightMouseButton)
				{
					//TODO: 서버 호출 함수로 변경
					ItemDragDrop->SourceInventory->Server_AddItemAmountAtSlot(ItemDragDrop->SourceIndex, ItemData->Amount);
					//GetItemAtIndex(ItemDragDrop->SourceIndex).Amount += ItemData->Amount;
					ItemDragDrop->SourceInventory->OnInventoryUpdated.Broadcast();
				}
				return false;
			}
		}
		
		//우클릭이면 떨구기면 반갈한 것만 버리기
		if (InDragDropEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			UE_LOG(LogTemp, Warning, TEXT("RIGHT CLICK DROP"));
			//TODO: 서버 호출 함수로 변경
			PlayerRef->Server_DropItem(
				ItemDragDrop->SourceInventory, ItemDragDrop->SourceIndex, ItemData->Amount);
			//DropItem(ItemData, ItemData->Amount, false);
			return true;
		}
		
		//좌클릭 떨구기면 싹다 버리기
		UE_LOG(LogTemp, Warning, TEXT("LEFT CLICK DROP"));
		//TODO: 서버 호출 함수로 변경
		PlayerRef->Server_DropItem(
			ItemDragDrop->SourceInventory, ItemDragDrop->SourceIndex, ItemData->Amount);
		//DropItem(ItemData, ItemData->Amount, true);
		return true;
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("DROP ON PANEL ITEM INVALID"));
	}
	return false;
}

void UInventory::HandleInventoryClicked()
{
	PanelSwitcher->SetActiveWidget(InventorySection);
	OnInventoryClicked.ExecuteIfBound();
}

void UInventory::HandleFoodRecipeClicked()
{
	PanelSwitcher->SetActiveWidget(FoodRecipeSection);
	if (UCraftPanel* FoodPanel = Cast<UCraftPanel>(FoodRecipeSection))
	{
		FoodPanel->SetCraftingMethod(ECraftMethod::INVEN);
	}
	OnCraftClicked.ExecuteIfBound();
}

void UInventory::HandleCraftRecipeClicked()
{
	PanelSwitcher->SetActiveWidget(CraftRecipeSection);
	if (UCraftPanel* CraftPanel = Cast<UCraftPanel>(CraftRecipeSection))
	{
		CraftPanel->SetCraftingMethod(ECraftMethod::INVEN);
	}
	OnCraftClicked.ExecuteIfBound();
}

void UInventory::HandleUnknownRecordClicked()
{
	PanelSwitcher->SetActiveWidget(UnknownRecordSection);
	OnUnknownRecordClicked.ExecuteIfBound();
}

