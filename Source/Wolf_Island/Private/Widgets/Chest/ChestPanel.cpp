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
}

void UChestPanel::NativeConstruct()
{
	Super::NativeConstruct();
}

void UChestPanel::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

bool UChestPanel::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UChestPanel::SetInventoryComponent(AChest* Chest, AActor* Interactor)
{
	//상자 인벤토리 설정
	ChestInventoryRef = Chest->InventoryComponent;
	//연 플레이어 설정
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

void UChestPanel::RefreshChest()
{
	UE_LOG(LogTemp, Warning, TEXT("RefreshChest"));
	if (ChestInventoryRef && SlotClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s 's Chest | Comp : %s | HasAuth? : %hs"), *ChestInventoryRef->GetOwner()->GetName(), *ChestInventoryRef->GetName(), ChestInventoryRef->GetOwner()->HasAuthority()?"SERVER":"CLIENT");
		ChestPanel->ClearChildren();

		int32 Index = 0;
		for (FItemSlot& InventorySlot : ChestInventoryRef->GetInventory())
		{
			UInventorySlot* ItemSlot = CreateWidget<UInventorySlot>(this, SlotClass);
			ItemSlot->SetIndex(Index++);
			ItemSlot->SetOwner(PlayerRef);
			ItemSlot->SetInventoryRef(ChestInventoryRef);
			
			ChestPanel->AddChildToWrapBox(ItemSlot);
			
			ItemSlot->RefreshSlot();
		}
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("RefreshChest isn't work"));
		if (!ChestInventoryRef) UE_LOG(LogTemp, Warning, TEXT("ChestInventoryRef isn't valid"));
		if (!SlotClass) UE_LOG(LogTemp, Warning, TEXT("SlotClass isn't valid"));
	}
}