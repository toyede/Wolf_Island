// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Chest/ChestScreen.h"

#include "Components/InventoryComponent.h"
#include "Interaction/Chest.h"
#include "Item/ItemBase.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Chest/ChestPanel.h"
#include "Widgets/Inventory/ItemDragDropOperation.h"

void UChestScreen::NativeConstruct()
{
	Super::NativeConstruct();

	FInputModeUIOnly UIOnly;
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PC->SetInputMode(UIOnly);
	PC->bShowMouseCursor = true;
	
	SetKeyboardFocus();
}

bool UChestScreen::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	const UItemDragDropOperation* ItemDragDrop = Cast<UItemDragDropOperation>(InOperation);

	UItemBase* Item = ItemDragDrop->SourceItem;
	FItemBaseData ItemData = ItemDragDrop->SourceItemData;
	
	UE_LOG(LogTemp, Warning, TEXT("CHEST SCREEN DROP DETECTED"));
	
	if (PlayerRef)
	{
		UE_LOG(LogTemp, Warning, TEXT("PLAYER AND ITEM IS VALID"));

		if (ChestPanel)
		{
			UE_LOG(LogTemp, Warning, TEXT("CHEST PANEL IS VALID"));

			FVector2D LocalMousePos = InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
			
			FGeometry PanelGeometry = ChestPanel->GetCachedGeometry();
			FVector2D PanelPos = InGeometry.AbsoluteToLocal(PanelGeometry.GetAbsolutePosition());
			FVector2D PanelSize = PanelGeometry.GetLocalSize();

			// 마우스가 패널 영역 안에 있다면 드랍 무시
			if (LocalMousePos.X >= PanelPos.X && LocalMousePos.X <= PanelPos.X + PanelSize.X &&
				LocalMousePos.Y >= PanelPos.Y && LocalMousePos.Y <= PanelPos.Y + PanelSize.Y)
			{
				UE_LOG(LogTemp, Warning, TEXT("DROPPED IN PANEL"));
				//우클릭이면 떨구기면 반갈한 거 원위치
				if (InDragDropEvent.GetEffectingButton() == EKeys::RightMouseButton)
				{
					ItemDragDrop->SourceInventory->GetItemAtIndex(ItemDragDrop->SourceIndex).Amount += Item->Amount;
					ItemDragDrop->SourceInventory->OnInventoryUpdated.Broadcast();
					ChestRef->InventoryComponent->OnInventoryUpdated.Broadcast();
				}
				return false;
			}
		}

		//우클릭이면 떨구기면 반갈한 것만 버리기
		if (InDragDropEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			UE_LOG(LogTemp, Warning, TEXT("RIGHT CLICK DROP"));
			PlayerRef->DropItem(ItemData, Item->Amount, false);
			return true;
		}
		
		UE_LOG(LogTemp, Warning, TEXT("LEFT CLICK DROP"));
		PlayerRef->DropItem(ItemData, Item->Amount, true);
		return true;
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("PLAYER OR ITEM NULL"));
	}
	
	return false;
}

bool UChestScreen::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	//UE_LOG(LogTemp, Warning, TEXT("CHEST SCREEN DRAG OVER DETECTED"));
	return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
}


FReply UChestScreen::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape || InKeyEvent.GetKey() == EKeys::Tab)
	{
		CloseWidget();
	}
	
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UChestScreen::InitializeChest(AChest* Chest, AActor* Interactor)
{
	ChestRef = Chest;
	PlayerRef = Cast<AMainPlayer>(Interactor);
	ChestPanel->SetInventoryComponent(Chest, Interactor);
	ChestPanel->RefreshChest();

}

void UChestScreen::CloseWidget()
{
	FInputModeGameOnly GameOnly;
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PC->SetInputMode(GameOnly);
	PC->bShowMouseCursor = false;
	ChestRef->IsOccupied = false;
	PlayerRef = nullptr;
	RemoveFromParent();
}
