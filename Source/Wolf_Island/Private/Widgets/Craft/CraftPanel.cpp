// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Craft/CraftPanel.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/InventoryComponent.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "Data/ItemDataStruct.h"
#include "Widgets/Craft/RecipeBlock.h"
#include "Widgets/Craft//CraftSlot.h"

void UCraftPanel::NativeConstruct()
{
	Super::NativeConstruct();

	OwnerInventory = GetOwningPlayerPawn()->GetComponentByClass<UInventoryComponent>();
	CraftButton->OnClicked.AddDynamic(this, &UCraftPanel::OnCraftButtonClicked);

	RecipeTable->ForeachRow<FRecipeData>(TEXT("RecipeTableContext"),
	[&](const FName& RowName, const FRecipeData& Recipe)
	{
		AddRecipe(Recipe);
	});
}

void UCraftPanel::OnCraftButtonClicked()
{
	if (OwnerInventory)
	{
		OwnerInventory->MakeItem(CurrentRecipeData);
		SetCraftButton(CurrentRecipeData);
	}
}

void UCraftPanel::AddRecipe(FRecipeData Recipe)
{
	if (RecipeBlockClass)
	{
		URecipeBlock* Block = CreateWidget<URecipeBlock>(GetWorld(), RecipeBlockClass);

		FItemData* ResultItem = ItemDataTable->FindRow<FItemData>(Recipe.ResultID, Recipe.ResultID.ToString());
		
		if (ResultItem)
		{
			Block->RecipeName->SetText(ResultItem->TextData.Name);
			if (ResultItem->AssetData.Icon)
			{
				Block->ItemIcon->SetBrushFromTexture(ResultItem->AssetData.Icon);
			}else
			{
				Block->ItemIcon->SetVisibility(ESlateVisibility::Hidden);
			}
			Block->RecipeData = Recipe;
			Block->OnRecipeClicked.AddDynamic(this, &UCraftPanel::SetRecipeInfo);
		}

		RecipeList->AddChild(Block);
	}
}

void UCraftPanel::SetRecipeInfo(FRecipeData RecipeData)
{
	//재료 슬롯 초기화
	IngredientList->ClearChildren();

	//재료 슬롯 생성
	for (const TPair<FName, int32>& Ingredient : RecipeData.GetIngredients())
	{
		FItemData* ItemData = ItemDataTable->FindRow<FItemData>(Ingredient.Key, "Ingredients");
		UCraftSlot* CraftSlot = CreateWidget<UCraftSlot>(GetWorld(), SlotClass);

		UE_LOG(LogTemp, Warning, TEXT("%s : %d"), *FText::FromName(Ingredient.Key).ToString(), Ingredient.Value);
		CraftSlot->SetCraftSlot(*ItemData, Ingredient.Value);

		IngredientList->AddChild(CraftSlot);		
	}

	//결과물 아이템 데이터
	FItemData* ResultData = ItemDataTable->FindRow<FItemData>(RecipeData.ResultID, "ResultItem");
	
	//결과물 정보 세팅
	ResultSlot->SetCraftSlot(*ResultData, RecipeData.ResultAmount);
	ItemName->SetText(ResultData->TextData.Name);
	ItemDescription->SetText(ResultData->TextData.Description);

	//제작 소요 시간
	DurationText->SetText(FText::FromString(FString::Printf(TEXT("%.1fs"), RecipeData.Duration)));

	//만들 수 있나 체크 후 버튼 활성화 결정
	SetCraftButton(RecipeData);
}

void UCraftPanel::SetCraftButton(FRecipeData RecipeData)
{
	if (OwnerInventory->CheckCanMakeRecipe(RecipeData))
	{
		CurrentRecipeData = RecipeData;		
		CraftButton->SetIsEnabled(true);
	} else
	{
		CraftButton->SetIsEnabled(false);
	}
}