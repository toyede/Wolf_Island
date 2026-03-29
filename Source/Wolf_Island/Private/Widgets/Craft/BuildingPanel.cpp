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

	// 플레이어 참조 가져오기
	AMainPlayer* Player = Cast<AMainPlayer>(GetOwningPlayerPawn());
	if (!Player) return;

	RecipeList->ClearChildren();

	int32 Index = 0;
	RecipeTable->ForeachRow<FRecipeData>(TEXT("BuildingContext"),
	[&](const FName& RowName, const FRecipeData& Recipe)
	{
		// 플레이어가 해금한 레시피가 아니라면 건너뜀
		if (!Player->HasRecipe(RowName)) return;

		bool bTypeMatch = (Recipe.ItemType == EItemType::BUILDING);
		bool bMethodMatch = (Recipe.Method == TargetBuildMethod);
		
		if (bTypeMatch && bMethodMatch)
		{
			URecipeBlock* NewBlock = AddBuildingRecipe(Recipe);
			
			if (Index == 0)
			{
				CurrentRecipeData = Recipe;
				CurrentRecipeBlock = NewBlock;
				
				SetBuildingInfo(NewBlock, CurrentRecipeData);
			}
			
			Index++;
		}
	});
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

void UBuildingPanel::SetBuildingInfo(URecipeBlock* NewBlock, FRecipeData RecipeData)
{
	//선택된 버튼 강조 변경
	//원래 선택 됐던 거 강조 해제
	CurrentRecipeBlock->SetSelected(false);
	//선택된 버튼을 최신 거로 변경
	CurrentRecipeBlock = NewBlock;
	//최신 선택된 거 강조
	CurrentRecipeBlock->SetSelected(true);
	
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

URecipeBlock* UBuildingPanel::AddBuildingRecipe(FRecipeData Recipe)
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
		
		return Block;
	}
	
	return nullptr;
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
