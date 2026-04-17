// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Craft/BuildingPanel.h"

#include "Character/MainPlayerController.h"
#include "Components/BuildingComponent.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/InventoryComponent.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "Character/MainPlayer.h"
#include "GameFramework/HUD.h"
#include "Games/MainGameState.h"
#include "Widgets/PlayerHUD.h"
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
		if (OwnerBuildingComponent)
		{
			OwnerBuildingComponent->OnBuildingModeEnded.AddDynamic(this, &UBuildingPanel::OnBuildingFinished);
		}
	}

	if (AMainGameState* GS = GetWorld() ? GetWorld()->GetGameState<AMainGameState>() : nullptr)
	{
		GS->OnSharedRecipesChanged.AddDynamic(this, &UBuildingPanel::RefreshBuildingList);
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
	CurrentRecipeBlock->SetSelected(false);
	CurrentRecipeBlock = NewBlock;
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

	ResultSlot->SetCraftSlot(ResultData, RecipeData.ResultAmount);
	ItemName->SetText(ResultData->TextData.Name);
	ItemDescription->SetText(ResultData->TextData.Description);
	//제작 소요 시간
	DurationText->SetText(FText::FromString(FString::Printf(TEXT("%.1fs"), RecipeData.Duration)));

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

void UBuildingPanel::OnBuildingFinished()
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		// 인풋 모드 복구
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}

	if (AMainPlayer* Player = Cast<AMainPlayer>(GetOwningPlayerPawn()))
	{
		if (Player->HUD)
		{
			Player->HUD->SetVisibility(ESlateVisibility::HitTestInvisible); 
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
