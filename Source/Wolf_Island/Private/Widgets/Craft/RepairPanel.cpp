#include "Widgets/craft/RepairPanel.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/InventoryComponent.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "Data/ItemDataStruct.h"
#include "Widgets/Craft/RepairBlock.h"
#include "Widgets/Craft/CraftSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Interaction/Repair_Actor.h"

void URepairPanel::NativeConstruct()
{
    Super::NativeConstruct();

    if (OwnerInventory)
    {
        OwnerInventory->OnInventoryUpdated.AddUObject(this, &URepairPanel::RefreshRecipeList);
    }
    
    if (GetOwningPlayerPawn())
    {
        OwnerInventory = GetOwningPlayerPawn()->GetComponentByClass<UInventoryComponent>();
    }

    if (RepairButton)
    {
        RepairButton->OnClicked.AddDynamic(this, &URepairPanel::OnRepairButtonClicked);
    }


    RefreshRecipeList();
}

void URepairPanel::InitRepairPanel(class ARepair_Actor* InRepairActor)
{
    TargetRepairActor = InRepairActor;

    if (TargetRepairActor)
    {
        TargetRepairActor->OnRepairStatusChanged.AddUniqueDynamic(this, &URepairPanel::RefreshRecipeList);
        
        RefreshRecipeList();
    }
}

void URepairPanel::RefreshRecipeList()
{
    if (!RecipeList || !RepairRecipeTable) return;

    RecipeList->ClearChildren();
    
    RepairRecipeTable->ForeachRow<FRepairRecipeData>(TEXT("RepairTableContext"),
    [&](const FName& RowName, const FRepairRecipeData& Recipe)
    {
        AddRecipe(RowName, Recipe);
    });
}

void URepairPanel::AddRecipe(FName RowName, FRepairRecipeData Recipe)
{
    if (!RepairBlockClass || !RecipeList) return;

    URepairBlock* Block = CreateWidget<URepairBlock>(this, RepairBlockClass);
    if (!Block) return;
    
    Block->RowName = RowName;
    Block->RepairRecipeData = Recipe; 
    
    Block->TargetActor = TargetRepairActor;

    if (!Block->OnRepairBlockClicked.IsAlreadyBound(this, &URepairPanel::SetRepairInfo))
    {
        Block->OnRepairBlockClicked.AddDynamic(this, &URepairPanel::SetRepairInfo);
    }

    if (Block->RecipeName)
    {
        Block->RecipeName->SetText(FText::FromName(Recipe.RecipeName));
    }
    
    Block->RefreshBlockStatus();
    
    RecipeList->AddChild(Block);
}

void URepairPanel::SetRepairInfo(FName RowName, FRepairRecipeData RecipeData)
{
    CurrentRepairData = RecipeData;
    CurrentRowName = RowName; 

    if (!IngredientList) return;
    IngredientList->ClearChildren();

    TMap<FName, int32> Ingredients = RecipeData.GetIngredients();

    for (const TPair<FName, int32>& Ingredient : Ingredients)
    {
        FName IngredientID = Ingredient.Key;
        if (!ItemDataTable) break;

        FItemData* ItemData = ItemDataTable->FindRow<FItemData>(IngredientID, "RepairIngredient");
        if (!ItemData) continue;
        if (!SlotClass) break;

        UCraftSlot* MaterialSlot = CreateWidget<UCraftSlot>(GetWorld(), SlotClass);
        if (MaterialSlot)
        {
            MaterialSlot->SetCraftSlot(ItemData, Ingredient.Value);
            IngredientList->AddChild(MaterialSlot);
        }
    }
    
    if (RecipeNameText) RecipeNameText->SetText(FText::FromName(RecipeData.RecipeName));
    if (DescriptionText) DescriptionText->SetText(FText::FromString(TEXT("수리하려면 재료를 모으세요.")));
    if (DurationText) DurationText->SetText(FText::FromString(FString::Printf(TEXT("%.1f s"), RecipeData.Duration)));

    SetRepairButtonState(RecipeData);
}

void URepairPanel::SetRepairButtonState(FRepairRecipeData RecipeData)
{
    if (!OwnerInventory || !RepairButton || !TargetRepairActor) return;

    bool bIsAlreadyComplete = TargetRepairActor->IsRecipeComplete(CurrentRowName);

    if (bIsAlreadyComplete)
    {
        RepairButton->SetIsEnabled(false);
        if (RecipeNameText) RecipeNameText->SetText(FText::FromString(TEXT("수리 완료됨")));
        return; 
    }

    if (RepairCheckSound)
    {
        UGameplayStatics::PlaySound2D(this, RepairCheckSound);
    }

    FRecipeData TempRecipe;
    TempRecipe.Ingredient1ID = RecipeData.Ingredient1ID;
    TempRecipe.Ingredient1Amount = RecipeData.Ingredient1Amount;
    TempRecipe.Ingredient2ID = RecipeData.Ingredient2ID;
    TempRecipe.Ingredient2Amount = RecipeData.Ingredient2Amount;
    TempRecipe.Ingredient3ID = RecipeData.Ingredient3ID;
    TempRecipe.Ingredient3Amount = RecipeData.Ingredient3Amount;
    TempRecipe.ResultID = FName("None");
    TempRecipe.ResultAmount = 0;

    bool bHasIngredients = OwnerInventory->CheckCanMakeRecipe(TempRecipe);
    RepairButton->SetIsEnabled(bHasIngredients);
    
    if (RecipeNameText) RecipeNameText->SetText(FText::FromName(RecipeData.RecipeName));
}

void URepairPanel::OnRepairButtonClicked()
{
    if (!OwnerInventory) return;

    if (OwnerInventory && TargetRepairActor && !CurrentRowName.IsNone())
    {
        OwnerInventory->Request_RepairShip(CurrentRowName, CurrentRepairData, TargetRepairActor);

        if (RepairSound)
        {
            UGameplayStatics::PlaySound2D(this, RepairSound);
        }

        SetRepairButtonState(CurrentRepairData);
        
    }
}