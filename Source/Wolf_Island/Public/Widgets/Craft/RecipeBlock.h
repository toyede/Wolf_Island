// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemDataStruct.h"
#include "RecipeBlock.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecipeButtonClicked, FRecipeData, RecipeData);

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

	UFUNCTION()
	void OnRecipeButtonClicked();

protected:
	virtual void NativeConstruct() override;
	
};
