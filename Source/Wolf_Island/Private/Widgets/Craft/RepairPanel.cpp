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
        UE_LOG(LogTemp, Warning, TEXT("RepairPanel: 수리 대상 액터 연결됨 -> %s"), *TargetRepairActor->GetName());
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
    if (!RepairBlockClass) return;

    URepairBlock* Block = CreateWidget<URepairBlock>(GetWorld(), RepairBlockClass);
    if (!Block) return;
    
    Block->RowName = RowName;
    Block->RepairRecipeData = Recipe; 

    if (!Block->OnRepairBlockClicked.IsAlreadyBound(this, &URepairPanel::SetRepairInfo))
    {
        Block->OnRepairBlockClicked.AddDynamic(this, &URepairPanel::SetRepairInfo);
    }

    if (Block->RecipeName)
    {
        Block->RecipeName->SetText(FText::FromName(Recipe.RecipeName));
    }
    
    bool bIsActuallyComplete = false;
    
    if (TargetRepairActor)
    {
        bIsActuallyComplete = TargetRepairActor->IsRecipeComplete(RowName);
    }
    else
    {
        bIsActuallyComplete = Recipe.Complete; 
    }

    if (bIsActuallyComplete)
    {
        if (Block->RecipeButton) 
        {
            Block->RecipeButton->SetIsEnabled(false);
        }

        if (Block->RecipeName)
        {
            Block->RecipeName->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
        }
    }
    
    if (RecipeList)
    {
        RecipeList->AddChild(Block);
    }
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
            MaterialSlot->SetCraftSlot(*ItemData, Ingredient.Value);
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
    if (!OwnerInventory || !RepairButton) return;

    bool bIsAlreadyComplete = false;
    if (TargetRepairActor)
    {
        bIsAlreadyComplete = TargetRepairActor->IsRecipeComplete(CurrentRowName);
    }

    if (bIsAlreadyComplete)
    {
        RepairButton->SetIsEnabled(false);
        return;
    }

    if (RepairCheckSound)
    {
        UGameplayStatics::PlaySound2D(this, RepairCheckSound);
    }

    // 재료 확인 로직
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
}

void URepairPanel::OnRepairButtonClicked()
{
    if (!OwnerInventory) return;

    if (RepairSound)
    {
        UGameplayStatics::PlaySound2D(this, RepairSound);
    }
    FRepairRecipeData TempRecipe;
    TempRecipe.Ingredient1ID = CurrentRepairData.Ingredient1ID;
    TempRecipe.Ingredient1Amount = CurrentRepairData.Ingredient1Amount;
    TempRecipe.Ingredient2ID = CurrentRepairData.Ingredient2ID;
    TempRecipe.Ingredient2Amount = CurrentRepairData.Ingredient2Amount;
    TempRecipe.Ingredient3ID = CurrentRepairData.Ingredient3ID;
    TempRecipe.Ingredient3Amount = CurrentRepairData.Ingredient3Amount;
    TempRecipe.Ingredient4ID = CurrentRepairData.Ingredient4ID;
    TempRecipe.Ingredient4Amount = CurrentRepairData.Ingredient4Amount;

    OwnerInventory->RepairShip(TempRecipe);
    
    if (TargetRepairActor)
    {
        TargetRepairActor->MarkRecipeAsComplete(CurrentRowName);
        UE_LOG(LogTemp, Log, TEXT("수리 완료 요청 보냄: %s"), *CurrentRowName.ToString());
    }
    RefreshRecipeList();
    
    SetRepairButtonState(CurrentRepairData);
}