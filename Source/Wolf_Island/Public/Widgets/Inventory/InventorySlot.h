// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Data/ItemDataStruct.h"
#include "InventorySlot.generated.h"


class AMainPlayer;
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
class UProgressBar;

UCLASS()
class WOLF_ISLAND_API UInventorySlot : public UUserWidget
{
	GENERATED_BODY()
public:

	FORCEINLINE void SetItemReference(FItemBaseData ItemIn) { ItemRef = ItemIn; };
	FORCEINLINE void SetIndex(int32 InIndex) { Index = InIndex; };
	FORCEINLINE FItemBaseData GetItemReference() const { return ItemRef; };
	FORCEINLINE void SetDragDrop(bool CanDD) { CanDragDrop = CanDD; };
	FORCEINLINE void SetOwner(AActor* Owner) { OwnerActor = Owner; };
	FORCEINLINE void SetInventoryRef(UInventoryComponent* Inventory) { OwnerInventoryRef = Inventory; };
	FORCEINLINE UInventoryComponent* GetOwnerRef() const { return OwnerInventoryRef; };
	//빠른 이동 시 옮겨갈 반대편 인벤토리(상자<->플레이어). 없으면 빠른 이동 비활성
	FORCEINLINE void SetLinkedInventory(UInventoryComponent* Inventory) { LinkedInventoryRef = Inventory; };
	//더블클릭/Shift+우클릭 시 반대편 인벤토리 빈 슬롯으로 이동 시도
	void TryQuickMove();
	void SetSelectedSlot();
	void SetUnSelectedSlot();
	void SetEmptySlot();
	void RefreshSlot();
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
	FItemBaseData ItemRef;
	
	//UPROPERTY(VisibleAnywhere, Category = "Inventory Slot")
	FItemData* ItemData;
	
	UPROPERTY(VisibleAnywhere, Category = "Inventory Slot")
	AActor* OwnerActor;
	

	//위젯
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UBorder* ItemBorder;

	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UImage* ItemIcon;
	
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UTextBlock* ItemAmount;

	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UProgressBar* DurabilityBar = nullptr;

	UPROPERTY(VisibleAnywhere)
	bool CanDragDrop = true;

	UPROPERTY(VisibleAnywhere)
	UInventoryComponent* OwnerInventoryRef;

	//빠른 이동 대상 인벤토리(상자 화면에서만 세팅됨. 일반 인벤토리는 null)
	UPROPERTY(VisibleAnywhere)
	UInventoryComponent* LinkedInventoryRef = nullptr;

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};
