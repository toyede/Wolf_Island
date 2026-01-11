// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemSubsystem.h"

void UItemSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	static ConstructorHelpers::FObjectFinder<UDataTable> ItemTableObj(
		TEXT("/Game/item/DT_ItemData.DT_ItemData")
		);
	
	if (ItemTableObj.Succeeded())
	{
		ItemDataTable = ItemTableObj.Object;
	}
}

void UItemSubsystem::Deinitialize()
{
	Super::Deinitialize();
	
	ItemDataTable = nullptr;
}

FItemData* UItemSubsystem::GetItemData(const FName ItemID) const
{
	if (!ItemDataTable || ItemID.IsNone())
	{
		return nullptr;
	}

	return ItemDataTable->FindRow<FItemData>(ItemID, TEXT("ItemSubsystem"));
}
