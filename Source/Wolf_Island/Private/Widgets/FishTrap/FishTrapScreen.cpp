// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/FishTrap/FishTrapScreen.h"
#include "Widgets/FishTrap/FishTrapPanel.h"
#include "Interaction/FishTrap.h"
#include "Character/MainPlayer.h"
#include "Components/InventoryComponent.h"
#include "Widgets/Inventory/ItemDragDropOperation.h"
#include "Kismet/GameplayStatics.h"

void UFishTrapScreen::InitializeScreen(class AFishTrap* Trap, AActor* Interactor)
{
	FishTrapRef = Trap;
	PlayerRef = Cast<AMainPlayer>(Interactor);

	if (FishTrapPanel && Trap)
	{
		FishTrapPanel->InitializePanel(Trap, Interactor);
	}
}

void UFishTrapScreen::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true); 
	
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeUIOnly UIOnly;
		UIOnly.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(UIOnly);
		PC->bShowMouseCursor = true;
	}
	
	SetKeyboardFocus();
}

void UFishTrapScreen::CloseWidget()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameOnly GameOnly;
		PC->SetInputMode(GameOnly);
		PC->bShowMouseCursor = false;
	}
	
	if (FishTrapRef)
	{
		FishTrapRef->Server_CloseFishTrap(); 
	}
    
	RemoveFromParent();
}

FReply UFishTrapScreen::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape || InKeyEvent.GetKey() == EKeys::Tab)
	{
		CloseWidget();
		return FReply::Handled();
	}
	
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

bool UFishTrapScreen::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	const UItemDragDropOperation* ItemDragDrop = Cast<UItemDragDropOperation>(InOperation);
	if (!ItemDragDrop || !PlayerRef) return false;

	FItemBaseData ItemData = ItemDragDrop->SourceItemData;

	if (FishTrapPanel)
	{
		FVector2D LocalMousePos = InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
		FGeometry PanelGeometry = FishTrapPanel->GetCachedGeometry();
		FVector2D PanelPos = InGeometry.AbsoluteToLocal(PanelGeometry.GetAbsolutePosition());
		FVector2D PanelSize = PanelGeometry.GetLocalSize();

		if (LocalMousePos.X >= PanelPos.X && LocalMousePos.X <= PanelPos.X + PanelSize.X &&
			LocalMousePos.Y >= PanelPos.Y && LocalMousePos.Y <= PanelPos.Y + PanelSize.Y)
		{
			if (InDragDropEvent.GetEffectingButton() == EKeys::RightMouseButton)
			{
				ItemDragDrop->SourceInventory->Server_AddItemAmountAtSlot(ItemDragDrop->SourceIndex, ItemData.Amount);
			}
			return false;
		}
	}

	if (InDragDropEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		PlayerRef->Request_DropItem(ItemDragDrop->SourceInventory, ItemDragDrop->SourceIndex, ItemData.Amount, false);
		return true;
	}
	
	PlayerRef->Request_DropItem(ItemDragDrop->SourceInventory, ItemDragDrop->SourceIndex, ItemData.Amount, true);
	return true;
}

bool UFishTrapScreen::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
}
