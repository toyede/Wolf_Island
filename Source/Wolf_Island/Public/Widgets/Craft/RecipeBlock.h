// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RecipeBlock.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API URecipeBlock : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UTextBlock* RecipeName;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UImage* ItemIcon;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UButton* RecipeButton;
};
