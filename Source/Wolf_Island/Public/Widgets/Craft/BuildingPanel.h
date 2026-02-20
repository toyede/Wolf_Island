// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemDataStruct.h"
#include "BuildingPanel.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UBuildingPanel : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void RefreshBuildingList();
	void SetBuildingMethod(ECraftMethod NewMethod);

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	UDataTable* ItemDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	UDataTable* RecipeTable;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	UDataTable* BuildingDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class URecipeBlock> RecipeBlockClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UCraftSlot> SlotClass;

	UPROPERTY(VisibleAnywhere, Category = "State")
	FRecipeData CurrentRecipeData;

	UPROPERTY()
	class UInventoryComponent* OwnerInventory;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnBuildButtonClicked();

	UFUNCTION()
	void SetBuildingInfo(FRecipeData RecipeData);

	void AddBuildingRecipe(FRecipeData Recipe);

protected:
	UPROPERTY(meta = (BindWidget))
	class UScrollBox* RecipeList;

	UPROPERTY(meta = (BindWidget))
	class UWrapBox* IngredientList;

	UPROPERTY(meta = (BindWidget))
	class UButton* BuildButton;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ItemName;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ItemDescription;
	
	void SendDebugChat(FString Message);

private:

	UPROPERTY()
	class UBuildingComponent* OwnerBuildingComponent;

	ECraftMethod TargetBuildMethod;
};
