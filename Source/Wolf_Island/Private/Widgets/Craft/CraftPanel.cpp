// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Craft/CraftPanel.h"

#include "Character/MainPlayer.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/InventoryComponent.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "Data/ItemDataStruct.h"
#include "Games/MainGameState.h"
#include "Widgets/Craft/RecipeBlock.h"
#include "Widgets/Craft//CraftSlot.h"

void UCraftPanel::SetCraftingMethod(ECraftMethod NewMethod)
{
	TargetCraftMethod = NewMethod;
    
	RefreshRecipeList();
}

void UCraftPanel::NativeConstruct()
{
	Super::NativeConstruct();

	OwnerInventory = GetOwningPlayerPawn()
		? GetOwningPlayerPawn()->GetComponentByClass<UInventoryComponent>()
		: nullptr;
	if (OwnerInventory)
	{
		OwnerInventory->OnInventoryUpdated.AddUObject(this, &UCraftPanel::RefreshRecipeList);
	}

	if (AMainGameState* GS = GetWorld() ? GetWorld()->GetGameState<AMainGameState>() : nullptr)
	{
		GS->OnSharedRecipesChanged.AddDynamic(this, &UCraftPanel::RefreshRecipeList);
	}
	CraftButton->OnClicked.AddDynamic(this, &UCraftPanel::OnCraftButtonClicked);

	RefreshRecipeList();
}

void UCraftPanel::OnCraftButtonClicked()
{
	if (OwnerInventory && CurrentRecipeData.ResultID != NAME_None)
	{
		if (AMainPlayer* Player = Cast<AMainPlayer>(OwnerInventory->GetOwner()))
		{
			Player->StartCraft(CurrentRecipeData);
		}

		SetCraftButton(CurrentRecipeData);
	}
}

URecipeBlock* UCraftPanel::AddRecipe(FRecipeData Recipe)
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
		return Block;
	}
	
	return nullptr;
}

inline void UCraftPanel::RefreshRecipeList()
{
	if (!RecipeList || !RecipeTable || !OwnerInventory) return;

	// 플레이어 참조 가져오기
	AMainPlayer* Player = Cast<AMainPlayer>(OwnerInventory->GetOwner());
	if (!Player) return;

	RecipeList->ClearChildren();

	int32 Index = 0;
	RecipeTable->ForeachRow<FRecipeData>(TEXT("RecipeTableContext"),
	[&](const FName& RowName, const FRecipeData& Recipe)
	{
		// 플레이어가 해금한 레시피가 아니라면 건너뜀
		if (!Player->HasRecipe(RowName)) return;

		// 1. 아이템 타입 필터
		bool bTypeMatch = RecipeTypeList.Contains(Recipe.ItemType) && (Recipe.ItemType != EItemType::BUILDING);
		bool bMethodMatch = (Recipe.Method == TargetCraftMethod);

		// 두 조건 다 맞으면 목록에 추가
		if (bTypeMatch && bMethodMatch)
		{
			URecipeBlock* NewBlock = AddRecipe(Recipe);
			
			//첫번째 레시피로 업데이트
			if (Index == 0)
			{
				CurrentRecipeData = Recipe;
				CurrentRecipeBlock = NewBlock;
				
				SetRecipeInfo(NewBlock, CurrentRecipeData);
			}
			
			Index++;
		}
	});
}

void UCraftPanel::SetRecipeInfo(URecipeBlock* ClickedBlock, FRecipeData RecipeData)
{
	//선택된 버튼 강조 변경
	//원래 선택 됐던 거 강조 해제
	CurrentRecipeBlock->SetSelected(false);
	//선택된 버튼을 최신 거로 변경
	CurrentRecipeBlock = ClickedBlock;
	//최신 선택된 거 강조
	CurrentRecipeBlock->SetSelected(true);
	
	//재료 슬롯 초기화
	IngredientList->ClearChildren();

	//재료 슬롯 생성
	for (const TPair<FName, int32>& Ingredient : RecipeData.GetIngredients())
	{
		FItemData* ItemData = ItemDataTable->FindRow<FItemData>(Ingredient.Key, "Ingredients");
		UCraftSlot* CraftSlot = CreateWidget<UCraftSlot>(GetWorld(), SlotClass);

		//UE_LOG(LogTemp, Warning, TEXT("%s : %d"), *FText::FromName(Ingredient.Key).ToString(), Ingredient.Value);
		CraftSlot->SetCraftSlot(ItemData, Ingredient.Value);

		IngredientList->AddChild(CraftSlot);		
	}

	//결과물 아이템 데이터
	FItemData* ResultData = ItemDataTable->FindRow<FItemData>(RecipeData.ResultID, "ResultItem");
	
	//결과물 정보 세팅
	ResultSlot->SetCraftSlot(ResultData, RecipeData.ResultAmount);
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
		CraftButton->SetIsEnabled(true);
		CurrentRecipeData = RecipeData;
	} else
	{
		CraftButton->SetIsEnabled(false);
	}
}
