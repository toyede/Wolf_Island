// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Craft/RepairBlock.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Styling/SlateColor.h"
#include "Interaction/Repair_Actor.h"

void URepairBlock::OnRepairButtonClicked()
{
	if (OnRepairBlockClicked.IsBound())
	{
		OnRepairBlockClicked.Broadcast(RowName, RepairRecipeData);
	}
}

void URepairBlock::RefreshBlockStatus()
{
	if (!TargetActor) return;

	bool bIsDone = TargetActor->IsRecipeComplete(RowName);

	if (bIsDone)
	{
		if (RecipeButton) 
		{
			RecipeButton->SetIsEnabled(false);
		}

		if (RecipeName)
		{
			RecipeName->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
		}
	}
	else
	{
		if (RecipeButton) RecipeButton->SetIsEnabled(true);
		if (RecipeName) RecipeName->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	}
}

void URepairBlock::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (RecipeButton)
	{
		RecipeButton->OnClicked.AddDynamic(this, &URepairBlock::OnRepairButtonClicked);
	}
	
}
