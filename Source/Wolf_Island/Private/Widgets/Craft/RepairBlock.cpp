// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Craft/RepairBlock.h"
#include "Components/Button.h"

void URepairBlock::OnRepairButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Block: 데이터 전송 시도 - Row: %s, Recipe: %s"), 
		*RowName.ToString(), 
		*RepairRecipeData.RecipeName.ToString());

	if (OnRepairBlockClicked.IsBound())
	{
		OnRepairBlockClicked.Broadcast(RowName, RepairRecipeData);
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
