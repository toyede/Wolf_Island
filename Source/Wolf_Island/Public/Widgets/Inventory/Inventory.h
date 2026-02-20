// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory.generated.h"

/**
 * 
 */

DECLARE_DELEGATE(FOnInventoryClicked);
DECLARE_DELEGATE(FOnCraftClicked);
DECLARE_DELEGATE(FOnUnknownRecordClicked);

UCLASS()
class WOLF_ISLAND_API UInventory : public UUserWidget
{
	GENERATED_BODY()

	FOnInventoryClicked OnInventoryClicked;

	FOnCraftClicked OnCraftClicked;

	FOnUnknownRecordClicked OnUnknownRecordClicked;

	UPROPERTY()
	class AMainPlayer* PlayerRef;

	UPROPERTY(meta=(BindWidget))
	class UWidgetSwitcher* PanelSwitcher;

	UPROPERTY(meta=(BindWidget))
	class USizeBox* InventorySection;
	
	UPROPERTY(meta=(BindWidget))
	USizeBox* FoodRecipeSection;

	UPROPERTY(meta=(BindWidget))
	USizeBox* CraftRecipeSection;

	UPROPERTY(meta=(BindWidget))
	USizeBox* UnknownRecordSection;

	UPROPERTY(meta=(BindWidget))
	USizeBox* BuildingRecipeSection;
	
	UPROPERTY(meta=(BindWidget))
	class UInventoryPanel* InventoryPanel;

	UPROPERTY(meta=(BindWidget))
	class UCraftPanel* FoodRecipePanel;

	UPROPERTY(meta=(BindWidget))
	class UCraftPanel* CraftRecipePanel;

	UPROPERTY(meta=(BindWidget))
	class UUnknownRecordPanel* UnknownRecordPanel;

	UPROPERTY(meta=(BindWidget))
	class UButton* InventoryButton;

	UPROPERTY(meta=(BindWidget))
	UButton* FoodRecipeButton;

	UPROPERTY(meta=(BindWidget))
	UButton* CraftRecipeButton;

	UPROPERTY(meta=(BindWidget))
	UButton* UnknownRecordButton;

	UPROPERTY(meta=(BindWidget))
	UButton* BuildingRecipeButton;

protected:
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UFUNCTION()
	void HandleInventoryClicked();
	UFUNCTION()
	void HandleFoodRecipeClicked();
	UFUNCTION()
	void HandleCraftRecipeClicked();
	UFUNCTION()
	void HandleUnknownRecordClicked();
	UFUNCTION()
	void HandleBuildingRecipeClicked();
};
