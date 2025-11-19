// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Craft/RecipeBlock.h"

#include "Components/Button.h"

void URecipeBlock::NativeConstruct()
{
	Super::NativeConstruct();

	if (RecipeButton)
	{
		RecipeButton->OnClicked.AddDynamic(this, &URecipeBlock::OnRecipeButtonClicked);
	}
	
}

void URecipeBlock::OnRecipeButtonClicked()
{
	OnRecipeClicked.Broadcast(RecipeData);
}
