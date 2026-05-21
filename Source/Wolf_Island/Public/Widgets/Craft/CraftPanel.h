// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemDataStruct.h"
#include "CraftPanel.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UCraftPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	UDataTable* ItemDataTable;
	UPROPERTY(EditDefaultsOnly)
	UDataTable* RecipeTable;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class URecipeBlock> RecipeBlockClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UCraftSlot> SlotClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UInventoryComponent* OwnerInventory;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRecipeData CurrentRecipeData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTimerHandle CraftingTimer;
	
	UPROPERTY(BlueprintReadWrite)
	URecipeBlock* CurrentRecipeBlock;


	UPROPERTY(EditAnywhere, meta=(BindWidget), BlueprintReadWrite)
	class UScrollBox* RecipeList;
	UPROPERTY(EditAnywhere, meta=(BindWidget), BlueprintReadWrite)
	class UTextBlock* ItemName;
	UPROPERTY(EditAnywhere, meta=(BindWidget), BlueprintReadWrite)
	UCraftSlot* ResultSlot;
	UPROPERTY(EditAnywhere, meta=(BindWidget), BlueprintReadWrite)
	UTextBlock* ItemDescription;
	UPROPERTY(EditAnywhere, meta=(BindWidget), BlueprintReadWrite)
	UTextBlock* DurationText;
	UPROPERTY(EditAnywhere, meta=(BindWidget), BlueprintReadWrite)
	class UWrapBox* IngredientList;
	UPROPERTY(EditAnywhere, meta=(BindWidget), BlueprintReadWrite)
	class UButton* CraftButton;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = " Setting")
	TArray<EItemType> RecipeTypeList;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = " Setting")
	ECraftMethod TargetCraftMethod = ECraftMethod::INVEN;
	
	UFUNCTION()
	URecipeBlock* AddRecipe(struct FRecipeData Recipe);
	UFUNCTION()
	void RefreshRecipeList();
	UFUNCTION(BlueprintCallable)
	void SetRecipeInfo(URecipeBlock* ClickedBlock, FRecipeData RecipeData);
	UFUNCTION(BlueprintCallable)
	void SetCraftButton(FRecipeData RecipeData);
	UFUNCTION(BlueprintCallable)
	void SetCraftingMethod(ECraftMethod NewMethod);
	UFUNCTION()
	void UpdateCraftUI();

protected:
	virtual void NativeConstruct() override;
	
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnCraftButtonClicked();
	
	bool StartCraft(FRecipeData RecipeData);
	
	void StopCraft();
	
	void MakeItem(FRecipeData RecipeData);
};
