// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemDataStruct.h"
#include "RecipeBlock.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRecipeButtonClicked, URecipeBlock*, ClickedBlock, FRecipeData, RecipeData);

UCLASS()
class WOLF_ISLAND_API URecipeBlock : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnRecipeButtonClicked OnRecipeClicked;

	UPROPERTY()
	FRecipeData RecipeData;
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UTextBlock* RecipeName;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UImage* ItemIcon;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UButton* RecipeButton;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UImage* SelectedIcon;

	UFUNCTION()
	void OnRecipeButtonClicked();
	
	UFUNCTION()
	void SetSelected(bool IsSelected);

protected:
	virtual void NativeConstruct() override;
	
};
