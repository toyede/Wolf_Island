// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UInventory : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY()
	class AMainPlayer* PlayerRef;
	
	UPROPERTY(meta=(BindWidget))
	class UInventoryPanel* InventoryPanel;

	UPROPERTY(meta=(BindWidget))
	class USizeBox* InventorySection;

protected:
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
};
