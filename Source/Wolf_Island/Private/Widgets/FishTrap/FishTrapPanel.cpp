// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/FishTrap/FishTrapPanel.h"
#include "Components/InventoryComponent.h"
#include "Components/WrapBox.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Interaction/FishTrap.h"
#include "Character/MainPlayer.h"
#include "Widgets/Inventory/InventorySlot.h"

void UFishTrapPanel::InitializePanel(AFishTrap* InTrap, AActor* InInteractor)
{
	TrapRef = InTrap;
	PlayerRef = Cast<AMainPlayer>(InInteractor);

	if (!SlotClass)
	{
		UClass* LoadedClass = StaticLoadClass(UInventorySlot::StaticClass(), nullptr, TEXT("/Game/JSY/Widgets/Inventory/WBP_InventorySlot.WBP_InventorySlot_C"));
		if (LoadedClass)
		{
			SlotClass = LoadedClass;
		}
	}

	if (PlayerRef)
	{   
		InventoryRef = PlayerRef->InventoryComponent;
		RefreshInventory(); 
	}
    
	if (TrapRef) 
	{
		TrapInventoryRef = TrapRef->InventoryComponent;
		if (TrapInventoryRef)
		{
			TrapInventoryRef->OnInventoryUpdated.AddUObject(this, &UFishTrapPanel::RefreshFishTrap);
		}
	}

	RefreshFishTrap();
}

void UFishTrapPanel::NativeDestruct()
{
	if (TrapInventoryRef) TrapInventoryRef->OnInventoryUpdated.RemoveAll(this);
	Super::NativeDestruct();
}

void UFishTrapPanel::RefreshFishTrap()
{
	if (!TrapInventoryRef || !SlotClass || !FishTrapSlotBox) return;

	FishTrapSlotBox->ClearChildren();
	int32 Index = 0;
    
	for (FItemSlot& TrapSlotData : TrapInventoryRef->GetInventory())
	{
		if (Index >= 10) break;
        
		UInventorySlot* ItemSlot = CreateWidget<UInventorySlot>(this, SlotClass);
		if (ItemSlot)
		{
			ItemSlot->SetIndex(Index++);
			ItemSlot->SetOwner(PlayerRef);
			ItemSlot->SetInventoryRef(TrapInventoryRef);
			FishTrapSlotBox->AddChildToWrapBox(ItemSlot);
			ItemSlot->RefreshSlot();
		}
	}

	if (BaitSlot && TrapInventoryRef->GetInventory().IsValidIndex(10))
	{
		BaitSlot->SetIndex(10);
		BaitSlot->SetOwner(PlayerRef);
		BaitSlot->SetInventoryRef(TrapInventoryRef);
		BaitSlot->RefreshSlot();
	}
}

void UFishTrapPanel::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (TrapRef)
	{
		if (FishTimerText) FishTimerText->SetText(FText::AsNumber(FMath::CeilToInt(TrapRef->GetRemainingFishTime())));
		if (BaitProgressBar) BaitProgressBar->SetPercent(TrapRef->GetRemainingBaitTime() / 60.0f);
	}
}

bool UFishTrapPanel::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}