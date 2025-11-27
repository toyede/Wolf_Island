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
	UPROPERTY(EditAnywhere)
	class UInventoryComponent* OwnerInventory;
	UPROPERTY(EditAnywhere)
	FRecipeData CurrentRecipeData;


	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	class UScrollBox* RecipeList;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	class UTextBlock* ItemName;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UCraftSlot* ResultSlot;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UTextBlock* ItemDescription;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	UTextBlock* DurationText;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	class UWrapBox* IngredientList;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	class UButton* CraftButton;
	UPROPERTY(EditAnywhere)
	TArray<EItemType> RecipeTypeList;

	
	UFUNCTION()
	void AddRecipe(struct FRecipeData Recipe);
	UFUNCTION()
	void RefreshRecipeList();
	UFUNCTION(BlueprintCallable)
	void SetRecipeInfo(FRecipeData RecipeData);
	UFUNCTION(BlueprintCallable)
	void SetCraftButton(FRecipeData RecipeData);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	ECraftMethod TargetCraftMethod = ECraftMethod::INVEN;
	
	UFUNCTION(BlueprintCallable)
	void SetCraftingMethod(ECraftMethod NewMethod);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnCraftButtonClicked();
};
