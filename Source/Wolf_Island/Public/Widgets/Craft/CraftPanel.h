// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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

	UPROPERTY(EditAnywhere, meta=(BindWidget))
	class UTextBlock* ItemName;
	UPROPERTY(EditAnywhere, meta=(BindWidget))
	class UTextBlock* ItemDescription;
	UPROPERTY(EditAnywhere, meta=(BindWidget))
	class UTextBlock* DurationText;
	UPROPERTY(EditAnywhere, meta=(BindWidget))
	class UScrollBox* RecipeList;
	UPROPERTY(EditAnywhere, meta=(BindWidget))
	class UWrapBox* Ingredients;
	UPROPERTY(EditAnywhere, meta=(BindWidget))
	class UButton* CraftButton;
	UPROPERTY(EditAnywhere, meta=(BindWidget))
	class UImage* ItemIcon;
	
	
};
