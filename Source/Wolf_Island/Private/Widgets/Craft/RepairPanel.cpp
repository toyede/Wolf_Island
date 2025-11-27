// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/craft/RepairPanel.h"
// 필요한 헤더들 (경로 확인 필요)
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/InventoryComponent.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "Data/ItemDataStruct.h"
#include "Widgets/Craft/RepairBlock.h"
#include "Widgets/Craft/CraftSlot.h"
#include "Interaction/Repair_Actor.h" // ARepair_Actor 헤더 포함 필수

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
        // 모든 레시피를 추가 (필요시 여기서 필터링 가능)
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
            // ★ 여기가 가장 의심스러운 부분!
            UE_LOG(LogTemp, Error, TEXT("!!! [찾기 실패] 아이템 테이블에 '%s' 라는 ID가 없습니다! 오타 확인 필요."), *IngredientID.ToString());
            continue; // 다음 재료로 넘어감
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

    // [수정] 하나씩 직접 대입해야 합니다.
    FRecipeData TempRecipe;
    
    TempRecipe.Ingredient1ID = RecipeData.Ingredient1ID;
    TempRecipe.Ingredient1Amount = RecipeData.Ingredient1Amount;
    
    TempRecipe.Ingredient2ID = RecipeData.Ingredient2ID;
    TempRecipe.Ingredient2Amount = RecipeData.Ingredient2Amount;
    
    TempRecipe.Ingredient3ID = RecipeData.Ingredient3ID;
    TempRecipe.Ingredient3Amount = RecipeData.Ingredient3Amount;

    // 주의: RepairData는 재료가 4개까지 있는데, RecipeData는 3개까지만 담을 수 있습니다.
    // 만약 4번째 재료가 필수라면, RecipeData 구조체도 4개로 늘려야 정확합니다.

    // 결과물은 없음
    TempRecipe.ResultID = FName("None");
    TempRecipe.ResultAmount = 0;

    // 인벤토리 함수 호출
    bool bHasIngredients = OwnerInventory->CheckCanMakeRecipe(TempRecipe);
    
    RepairButton->SetIsEnabled(bHasIngredients);
}

void URepairPanel::OnRepairButtonClicked()
{
    if (!OwnerInventory) return;

    // [수정] 변환 과정 (하나씩 대입)
    FRecipeData TempRecipe;
    
    TempRecipe.Ingredient1ID = CurrentRepairData.Ingredient1ID;
    TempRecipe.Ingredient1Amount = CurrentRepairData.Ingredient1Amount;
    
    TempRecipe.Ingredient2ID = CurrentRepairData.Ingredient2ID;
    TempRecipe.Ingredient2Amount = CurrentRepairData.Ingredient2Amount;
    
    TempRecipe.Ingredient3ID = CurrentRepairData.Ingredient3ID;
    TempRecipe.Ingredient3Amount = CurrentRepairData.Ingredient3Amount;

    TempRecipe.ResultID = FName("None");
    TempRecipe.ResultAmount = 0;

    // 1. 인벤토리에서 재료 소모
    OwnerInventory->MakeItem(TempRecipe);

    // 2. 액터 상태 변경
    if (TargetRepairActor)
    {
        TargetRepairActor->CompleteRepair();
    }

    // 3. 데이터 테이블 변경
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