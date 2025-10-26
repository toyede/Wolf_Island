// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventorySlot.generated.h"

/**
 * 
 */
class UItemBase;
class UDragItemVisual;
class UInventoryToolTip;
class UTextBlock;
class UBorder;
class UImage;

UCLASS()
class WOLF_ISLAND_API UInventorySlot : public UUserWidget
{
	GENERATED_BODY()
public:

	FORCEINLINE void SetItemReference(UItemBase* ItemIn) { ItemRef = ItemIn; };
	FORCEINLINE UItemBase* GetItemReference() const { return ItemRef; };

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Inventory Slot")
	TSubclassOf<UDragItemVisual> DragItemVisualClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory Slot")
	TSubclassOf<UInventoryToolTip> ToolTipClass;
	
	UPROPERTY(VisibleAnywhere, Category="Inventory Slot")
	UItemBase* ItemRef;

	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UBorder* ItemBorder;

	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UImage* ItemIcon;
	
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UTextBlock* ItemAmount;

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
