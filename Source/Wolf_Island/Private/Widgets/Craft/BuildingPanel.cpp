// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Craft/BuildingPanel.h"

#include "Components/BuildingComponent.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/InventoryComponent.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "Games/MainGameState.h"
#include "Widgets/Craft/CraftSlot.h"
#include "Widgets/Craft/RecipeBlock.h"
#include "Widgets/Inventory/Inventory.h"

void UBuildingPanel::RefreshBuildingList()
{
	if (!RecipeList || !RecipeTable) return;

	RecipeList->ClearChildren();

	int32 Index = 0;
	RecipeTable->ForeachRow<FRecipeData>(TEXT("BuildingContext"),
	[&](const FName& RowName, const FRecipeData& Recipe)
	{
		bool bTypeMatch = (Recipe.ItemType == EItemType::BUILDING);
        
		bool bMethodMatch = (Recipe.Method == TargetBuildMethod);
		
		if (bTypeMatch && bMethodMatch)
		{
			if (Index == 0)
			{
				CurrentRecipeData = Recipe;
			}
			
			AddBuildingRecipe(Recipe);
			Index++;
		}
	});
	
	SetBuildingInfo(CurrentRecipeData);
}

void UBuildingPanel::SetBuildingMethod(ECraftMethod NewMethod)
{
	TargetBuildMethod = NewMethod;
    
	RefreshBuildingList();
}

void UBuildingPanel::NativeConstruct()
{
	Super::NativeConstruct();

	APawn* Pawn = GetOwningPlayerPawn();
	if (Pawn)
	{
		OwnerInventory = Pawn->GetComponentByClass<UInventoryComponent>();
		OwnerBuildingComponent = Pawn->GetComponentByClass<UBuildingComponent>();
	}

	if (BuildButton)
	{
		BuildButton->OnClicked.AddDynamic(this, &UBuildingPanel::OnBuildButtonClicked);
	}

	RefreshBuildingList();
}

void UBuildingPanel::OnBuildButtonClicked()
{
	if (OwnerBuildingComponent && CurrentRecipeData.ResultID != NAME_None)
	{
		if (!BuildingDataTable) 
		{
			SendDebugChat(TEXT("BuildingDataTable이 비어있습니다!"));
			return;
		}

		FBuildingData* BuildData = BuildingDataTable->FindRow<FBuildingData>(CurrentRecipeData.ResultID, "");
        
		if (BuildData)
		{
			OwnerBuildingComponent->EnterBuildMode(CurrentRecipeData, *BuildData);

			APlayerController* PC = GetOwningPlayer();
			if (PC)
			{
				FInputModeGameOnly InputMode;
				PC->SetInputMode(InputMode);
				PC->bShowMouseCursor = false;
			}
			UInventory* ParentInventory = Cast<UInventory>(GetTypedOuter<UInventory>());
			if (ParentInventory)
			{
				ParentInventory->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
		else 
		{
			SendDebugChat(TEXT("ID를 BuildingDataTable에서 찾을 수 없습니다."));
		}
	}
}

void UBuildingPanel::SetBuildingInfo(FRecipeData RecipeData)
{
	CurrentRecipeData = RecipeData;
	IngredientList->ClearChildren();

	for (const TPair<FName, int32>& Ingredient : RecipeData.GetIngredients())
	{
		FItemData* ItemData = ItemDataTable->FindRow<FItemData>(Ingredient.Key, "");
		UCraftSlot* IngredientSlot = CreateWidget<UCraftSlot>(GetWorld(), SlotClass);
		IngredientSlot->SetCraftSlot(ItemData, Ingredient.Value);
		IngredientList->AddChild(IngredientSlot);		
	}

	FItemData* ResultData = ItemDataTable->FindRow<FItemData>(RecipeData.ResultID, "");
	ItemName->SetText(ResultData->TextData.Name);
	ItemDescription->SetText(ResultData->TextData.Description);

	if (OwnerInventory && OwnerInventory->CheckCanMakeRecipe(RecipeData))
	{
		BuildButton->SetIsEnabled(true);
	}
	else
	{
		BuildButton->SetIsEnabled(false);
	}
}

void UBuildingPanel::AddBuildingRecipe(FRecipeData Recipe)
{
	if (RecipeBlockClass)
	{
		URecipeBlock* Block = CreateWidget<URecipeBlock>(GetWorld(), RecipeBlockClass);
		FItemData* ResultItem = ItemDataTable->FindRow<FItemData>(Recipe.ResultID, "");
		
		if (ResultItem)
		{
			Block->RecipeName->SetText(ResultItem->TextData.Name);
			Block->ItemIcon->SetBrushFromTexture(ResultItem->AssetData.Icon);
			Block->RecipeData = Recipe;
			Block->OnRecipeClicked.AddDynamic(this, &UBuildingPanel::SetBuildingInfo);
			RecipeList->AddChild(Block);
		}
	}
}

void UBuildingPanel::SendDebugChat(FString Message)
{
	if (AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>())
	{
		FChattingData DebugMsg;
		DebugMsg.Name = TEXT("System_Debug");
		DebugMsg.Message = Message;
		GS->AddChattingMessage(DebugMsg);
	}
}
