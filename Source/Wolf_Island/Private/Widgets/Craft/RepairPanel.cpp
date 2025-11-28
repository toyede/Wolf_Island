// Fill out your copyright notice in the Description page of Project Settings.


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
#include "Interaction/Repair_Actor.h"

void URepairPanel::NativeConstruct()
{
    Super::NativeConstruct();
    // 플레이어의 인벤토리 가져오기
    if (GetOwningPlayerPawn())
    {
        OwnerInventory = GetOwningPlayerPawn()->GetComponentByClass<UInventoryComponent>();
    }

    // 버튼 이벤트 연결
    if (RepairButton)
    {
        RepairButton->OnClicked.AddDynamic(this, &URepairPanel::OnRepairButtonClicked);
    }

    // 초기 목록 갱신
    RefreshRecipeList();
}

void URepairPanel::RefreshRecipeList()
{
    if (!RecipeList || !RepairRecipeTable) return;

    RecipeList->ClearChildren();
    
    // 데이터 테이블 순회
    RepairRecipeTable->ForeachRow<FRepairRecipeData>(TEXT("RepairTableContext"),
    [&](const FName& RowName, const FRepairRecipeData& Recipe)
    {
        // 모든 레시피를 추가
        AddRecipe(RowName, Recipe);
    });
}

void URepairPanel::AddRecipe(FName RowName, FRepairRecipeData Recipe)
{
    if (!RepairBlockClass)
    {
        UE_LOG(LogTemp, Error, TEXT("RepairPanel: RepairBlockClass가 비어있습니다!"));
        return;
    }

    URepairBlock* Block = CreateWidget<URepairBlock>(GetWorld(), RepairBlockClass);
    if (!Block)
    {
        UE_LOG(LogTemp, Error, TEXT("RepairPanel: 위젯 생성 실패!"));
        return;
    }
    
    Block->RowName = RowName;
    Block->RepairRecipeData = Recipe; 

    UE_LOG(LogTemp, Warning, TEXT("Panel -> Block 데이터 주입: %s"), *Recipe.RecipeName.ToString());

    if (!Block->OnRepairBlockClicked.IsAlreadyBound(this, &URepairPanel::SetRepairInfo))
    {
        Block->OnRepairBlockClicked.AddDynamic(this, &URepairPanel::SetRepairInfo);
        UE_LOG(LogTemp, Log, TEXT(">>> [연결 성공] Block과 Panel 연결됨 (%s)"), *RowName.ToString());
    }

    if (Block->RecipeName)
    {
        Block->RecipeName->SetText(FText::FromName(Recipe.RecipeName));
    }

    
    if (Recipe.Complete)
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
    // 1. 함수 진입 확인
    UE_LOG(LogTemp, Warning, TEXT("=========================================="));
    UE_LOG(LogTemp, Warning, TEXT("[SetRepairInfo] 레시피 선택됨: %s"), *RowName.ToString());

    CurrentRepairData = RecipeData;
    CurrentRowName = RowName; 

    // 2. 재료 슬롯 박스 확인
    if (!IngredientList)
    {
        UE_LOG(LogTemp, Error, TEXT("!!! [에러] IngredientList(가로/세로 박스)가 연결 안 됨!"));
        return;
    }

    IngredientList->ClearChildren();

    // 3. 재료 목록 가져오기
    TMap<FName, int32> Ingredients = RecipeData.GetIngredients();
    UE_LOG(LogTemp, Log, TEXT(">>> 필요한 재료 개수: %d 종류"), Ingredients.Num());

    if (Ingredients.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("!!! [경고] 이 레시피는 재료가 0개입니다. 데이터 테이블을 확인하세요."));
    }

    // 4. 재료 순회 시작
    for (const TPair<FName, int32>& Ingredient : Ingredients)
    {
        FName IngredientID = Ingredient.Key;
        int32 Amount = Ingredient.Value;

        UE_LOG(LogTemp, Log, TEXT("   - 검색 중인 재료 ID: %s (개수: %d)"), *IngredientID.ToString(), Amount);

        // 5. 아이템 데이터 테이블 검색
        if (!ItemDataTable)
        {
            UE_LOG(LogTemp, Error, TEXT("!!! [치명적] ItemDataTable이 None입니다! 디테일 패널 확인 필요."));
            break;
        }

        FItemData* ItemData = ItemDataTable->FindRow<FItemData>(IngredientID, "RepairIngredient");
        
        if (!ItemData)
        {
            UE_LOG(LogTemp, Error, TEXT("!!! [찾기 실패] 아이템 테이블에 '%s' 라는 ID가 없습니다! 오타 확인 필요."), *IngredientID.ToString());
            continue;
        }

        // 6. 슬롯 클래스 확인
        if (!SlotClass)
        {
            UE_LOG(LogTemp, Error, TEXT("!!! [에러] SlotClass가 None입니다! WBP_InventorySlot을 할당하세요."));
            break;
        }

        // 7. 위젯 생성
        UCraftSlot* MaterialSlot = CreateWidget<UCraftSlot>(GetWorld(), SlotClass);
        
        if (MaterialSlot)
        {
            // 데이터 세팅
            MaterialSlot->SetCraftSlot(*ItemData, Ingredient.Value);
            IngredientList->AddChild(MaterialSlot);
            UE_LOG(LogTemp, Warning, TEXT("   -> [성공] 슬롯 생성 완료: %s"), *IngredientID.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("!!! [에러] 위젯 생성 실패 (CreateWidget returned null)"));
        }
    }
    
    // 텍스트 및 버튼 갱신
    if (RecipeNameText) RecipeNameText->SetText(FText::FromName(RecipeData.RecipeName));
    if (DescriptionText) DescriptionText->SetText(FText::FromString(TEXT("수리하려면 재료를 모으세요.")));
    if (DurationText) DurationText->SetText(FText::FromString(FString::Printf(TEXT("%.1f s"), RecipeData.Duration)));

    SetRepairButtonState(RecipeData);
}

void URepairPanel::SetRepairButtonState(FRepairRecipeData RecipeData)
{
    if (!OwnerInventory || !RepairButton) return;

    if (RecipeData.Complete)
    {
        RepairButton->SetIsEnabled(false);
        return;
    }
    FRecipeData TempRecipe;
    
    TempRecipe.Ingredient1ID = RecipeData.Ingredient1ID;
    TempRecipe.Ingredient1Amount = RecipeData.Ingredient1Amount;
    
    TempRecipe.Ingredient2ID = RecipeData.Ingredient2ID;
    TempRecipe.Ingredient2Amount = RecipeData.Ingredient2Amount;
    
    TempRecipe.Ingredient3ID = RecipeData.Ingredient3ID;
    TempRecipe.Ingredient3Amount = RecipeData.Ingredient3Amount;
    
    /*
    TempRecipe.Ingredient4ID = RecipeData.Ingredient4ID;
    TempRecipe.Ingredient4Amount = RecipeData.Ingredient4Amount;
    */
    
    TempRecipe.ResultID = FName("None");
    TempRecipe.ResultAmount = 0;

    // 인벤토리 함수 호출
    bool bHasIngredients = OwnerInventory->CheckCanMakeRecipe(TempRecipe);
    
    RepairButton->SetIsEnabled(bHasIngredients);
}

void URepairPanel::OnRepairButtonClicked()
{
    if (!OwnerInventory) return;

    // 변환 과정
    FRepairRecipeData TempRecipe;
    
    TempRecipe.Ingredient1ID = CurrentRepairData.Ingredient1ID;
    TempRecipe.Ingredient1Amount = CurrentRepairData.Ingredient1Amount;
    
    TempRecipe.Ingredient2ID = CurrentRepairData.Ingredient2ID;
    TempRecipe.Ingredient2Amount = CurrentRepairData.Ingredient2Amount;
    
    TempRecipe.Ingredient3ID = CurrentRepairData.Ingredient3ID;
    TempRecipe.Ingredient3Amount = CurrentRepairData.Ingredient3Amount;

    TempRecipe.Ingredient4ID = CurrentRepairData.Ingredient4ID;
    TempRecipe.Ingredient4Amount = CurrentRepairData.Ingredient4Amount;


    // 1. 인벤토리에서 재료 소모
    OwnerInventory->RepairShip(TempRecipe);
    
    // 2. 액터 상태 변경
    if (TargetRepairActor)
    {
        UE_LOG(LogTemp, Error, TEXT("TargetRepairActor ON"));
        
        TargetRepairActor->CompleteRepair();
    }

    // 3. 데이터 테이블 수정
    MarkRecipeAsComplete(CurrentRowName);

    // 4. UI 갱신
    CurrentRepairData.Complete = true;
    RefreshRecipeList();
    SetRepairButtonState(CurrentRepairData);

    UE_LOG(LogTemp, Log, TEXT("수리 완료: %s"), *CurrentRepairData.RecipeName.ToString());
}

void URepairPanel::MarkRecipeAsComplete(FName RowName)
{
    if (!RepairRecipeTable) return;

    FString ContextString;
    // 테이블 원본 데이터 포인터 가져오기
    FRepairRecipeData* Row = RepairRecipeTable->FindRow<FRepairRecipeData>(RowName, ContextString);

    if (Row)
    {
        Row->Complete = true; // 런타임 값 수정
        UE_LOG(LogTemp, Log, TEXT("DataTable Updated: %s is now Complete."), *RowName.ToString());
    }
}

void URepairPanel::InitRepairPanel(class ARepair_Actor* InRepairActor)
{
    TargetRepairActor = InRepairActor;

    if (TargetRepairActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("RepairPanel: 수리 대상 액터 연결됨 -> %s"), *TargetRepairActor->GetName());
    }
}
