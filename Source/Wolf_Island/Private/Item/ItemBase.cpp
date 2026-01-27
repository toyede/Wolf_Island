// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemBase.h"
#include "Components/InventoryComponent.h"

UItemBase::UItemBase() : IsCopy(false), IsPickup(false)
{
}

UItemBase* UItemBase::CreateItemCopy() const
{
    UItemBase* ItemCopy = NewObject<UItemBase>(StaticClass());

    ItemCopy->ID = this->ID;
    ItemCopy->Amount = this->Amount;
    ItemCopy->Type = this->Type;
    ItemCopy->TextData = this->TextData;
    ItemCopy->NumericData = this->NumericData;
    ItemCopy->AssetData = this->AssetData;
    ItemCopy->IsCopy = true;

    return ItemCopy;
}

void UItemBase::SetAmount(const int32 NewAmount)
{
    if (NewAmount != Amount) {
        Amount = FMath::Clamp(NewAmount, 0, NumericData.IsStackable ? NumericData.MaxAmount : 1);

        if (OwningInventory) {
            if(Amount <= 0){
                //OwningInventory->RemoveSingleInstanceOfItem(this);
            }
        }
    }
}

void UItemBase::ResetItemFlags()
{
    IsCopy = false;
    IsPickup = false;
}

void UItemBase::Use_Implementation()
{
}
