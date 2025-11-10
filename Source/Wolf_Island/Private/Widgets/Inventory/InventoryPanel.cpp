// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/InventoryPanel.h"

#include "Components/InventoryComponent.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/WrapBox.h"
#include "Item/ItemBase.h"

void UInventoryPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	PlayerRef = Cast<AMainPlayer>(GetOwningPlayerPawn());
	if (PlayerRef)
	{
		InventoryRef = PlayerRef->InventoryComponent;
		
		if (InventoryRef)
		{
			InventoryRef->OnInventoryUpdated.AddUObject(this, &UInventoryPanel::RefreshInventory);
			SetInfoText();
			UE_LOG(LogTemp, Warning, TEXT("RefreshInventory Registered"));
		}
	}
}

void UInventoryPanel::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (InventoryRef)
	{
		WeightBar->SetPercent(InventoryRef->GetWeightPercent());
	}
}

void UInventoryPanel::SetInfoText() const
{
	//용량 ( ex) 2/100 ) 텍스트 업데이트
	const FString WeightInfoText =
		{FString::SanitizeFloat(InventoryRef->GetCurrentWeight())+"/"+
		 FString::FromInt(InventoryRef->GetWeightCapacity())};
	WeightInfo->SetText(FText::FromString(WeightInfoText));
}

void UInventoryPanel::RefreshInventory()
{
	UE_LOG(LogTemp, Warning, TEXT("RefreshInventory"));
	if (InventoryRef && SlotClass)
	{
		InventoryPanel->ClearChildren();
		HandSection->ClearChildren();
		
		int32 Index = 0;
		for (FItemSlot InventorySlot : InventoryRef->GetInventory())
		{
			UInventorySlot* ItemSlot = CreateWidget<UInventorySlot>(this, SlotClass);
			ItemSlot->SetIndex(Index++);
			
			if (InventorySlot.Item)
			{
				ItemSlot->SetItemReference(InventorySlot.Item);
			} 

			if (Index==1)
			{
				HandSection->AddChild(ItemSlot);
			} else
			{
				InventoryPanel->AddChildToWrapBox(ItemSlot);
			}
		}

		SetInfoText();
	}
}

bool UInventoryPanel::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}
