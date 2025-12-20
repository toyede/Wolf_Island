// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/HotbarSlot.h"

#include "Components/TextBlock.h"

void UHotbarSlot::SetSlotNumber(int32 SlotIndex)
{
    SlotNumber->SetText(FText::AsNumber(SlotIndex));
}
