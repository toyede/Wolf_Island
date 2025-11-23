// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Chest/ChestPanel.h"

#include "Components/InventoryComponent.h"
#include "Components/WrapBox.h"
#include "Data/ItemDataStruct.h"
#include "Interaction/Chest.h"
#include "Kismet/KismetSystemLibrary.h"

void UChestPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ChestInventoryRef)
	{
		ChestInventoryRef->OnInventoryUpdated.AddUObject(this, &UChestPanel::RefreshChest);
	}
	if (InventoryRef)
	{
		InventoryRef->OnInventoryUpdated.AddUObject(this, &UChestPanel::RefreshChest);
	}
}

void UChestPanel::NativeConstruct()
{
	Super::NativeConstruct();
}

void UChestPanel::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	FString Message = InventoryRef->GetOwner()->GetName();
	//UKismetSystemLibrary::PrintString(GetWorld(), Message, true, true, FLinearColor::Green, InDeltaTime);
}

bool UChestPanel::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
                               UDragDropOperation* InOperation)
{
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UChestPanel::RefreshChest()
{
	UE_LOG(LogTemp, Warning, TEXT("RefreshChest"));
	if (ChestInventoryRef && SlotClass)
	{
		ChestPanel->ClearChildren();

		int32 Index = 0;
		for (FItemSlot& InventorySlot : ChestInventoryRef->GetInventory())
		{
			UInventorySlot* ItemSlot = CreateWidget<UInventorySlot>(this, SlotClass);
			ItemSlot->SetIndex(Index++);
			ItemSlot->SetOwnerRef(ChestInventoryRef);
			
			if (InventorySlot.Item)
			{
				ItemSlot->SetItemReference(InventorySlot.Item);
			}
			
			ChestPanel->AddChildToWrapBox(ItemSlot);
			
		}
	}
	RefreshInventory();
}

void UChestPanel::SetInventoryComponent(AChest* Chest, AActor* Interactor)
{
	ChestInventoryRef = Chest->InventoryComponent;
	PlayerRef = Cast<AMainPlayer>(Interactor);
	
	if (PlayerRef)
	{
		InventoryRef = PlayerRef->InventoryComponent;
	} else
	{
		UE_LOG(LogTemp, Error, TEXT("CHEST PANEL : INTERACTOR PLAYER NOT FOUND"));
	}

	RefreshChest();
}