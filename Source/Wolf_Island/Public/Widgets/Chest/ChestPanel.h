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

protected:

	virtual void NativeOnInitialized() override;
	
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
};
