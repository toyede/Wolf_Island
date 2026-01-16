// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Data/ItemDataStruct.h"
#include "InventorySlot.generated.h"

class UInventoryComponent;
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

	FORCEINLINE void SetItemReference(FItemBaseData* ItemIn) { ItemRef = ItemIn; };
	FORCEINLINE void SetIndex(int32 InIndex) { Index = InIndex; };
	FORCEINLINE FItemBaseData* GetItemReference() const { return ItemRef; };
	FORCEINLINE void SetDragDrop(bool CanDD) { CanDragDrop = CanDD; };
	FORCEINLINE void SetOwnerRef(UInventoryComponent* Inventory) { OwnerInventoryRef = Inventory; };
	FORCEINLINE UInventoryComponent* GetOwnerRef() const { return OwnerInventoryRef; };
	void SetSelectedSlot();
	void SetUnSelectedSlot();
	void SetEmptySlot();
	FLinearColor GetBrushColor() const { return ItemBorder->GetBrushColor(); };

protected:

	//데이터
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "InventorySlot")
	int32 Index;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InventorySlot")
	FSlateBrush SelectedSlotBrush;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "InventorySlot")
	FSlateBrush UnSelectedSlotBrush;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory Slot")
	TSubclassOf<UDragItemVisual> DragItemVisualClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory Slot")
	TSubclassOf<UInventoryToolTip> ToolTipClass;
	
	//UPROPERTY(VisibleAnywhere, Category = "Inventory Slot")
	FItemBaseData* ItemRef;
	
	FItemData* ItemData;

	//위젯
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UBorder* ItemBorder;

	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UImage* ItemIcon;
	
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UTextBlock* ItemAmount;

	UPROPERTY(VisibleAnywhere)
	bool CanDragDrop = true;

	UPROPERTY(VisibleAnywhere)
	UInventoryComponent* OwnerInventoryRef;
	
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
