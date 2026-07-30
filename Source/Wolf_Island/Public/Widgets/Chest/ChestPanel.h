// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Inventory/InventoryPanel.h"
#include "ChestPanel.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UChestPanel : public UInventoryPanel
{
	GENERATED_BODY()

public:

	UPROPERTY(meta=(BindWidget))
	UWrapBox* ChestPanel;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Chest")
	UInventoryComponent* ChestInventoryRef;

	UFUNCTION()
	void SetInventoryComponent(class AChest* Chest, AActor* Interactor);

	UFUNCTION()
	void RefreshChest();

	//빠른 이동 대상: 플레이어 인벤 슬롯 -> 상자 인벤토리
	virtual UInventoryComponent* GetCounterpartInventory() const override { return ChestInventoryRef; }

protected:

	virtual void NativeOnInitialized() override;
	
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
};
