// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory.h"

#include "Character/MainPlayer.h"
#include "Item/ItemBase.h"
#include "Widgets/ItemDragDropOperation.h"

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

	if (ItemDragDrop)
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemDragDrop CASTING TRUE"));
	}else
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemDragDrop CASTING FALSE"));
	}

	UItemBase* Item = ItemDragDrop->SourceItem;

	if (Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("Source Item TRUE"));
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("Source Item FALSE"));
	}

	if (PlayerRef)
	{
		UE_LOG(LogTemp, Warning, TEXT(" Player TRUE"));
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT(" Player FALSE"));
	}
	
	if (PlayerRef && Item)
	{
		PlayerRef->DropItem(Item, Item->Amount);
		UE_LOG(LogTemp, Warning, TEXT("UInventory::NativeOnDrop TRUE"));
		return true;
	}
	UE_LOG(LogTemp, Warning, TEXT("UInventory::NativeOnDrop FALSE"));
	return false;
}

