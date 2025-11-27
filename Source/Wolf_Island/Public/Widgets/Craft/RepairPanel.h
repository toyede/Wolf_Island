// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemDataStruct.h" // FRepairRecipeData 구조체가 정의된 헤더
#include "RepairPanel.generated.h"

class ARepair_Actor;
class UInventoryComponent;

/**
 * 수리 패널 위젯
 */
UCLASS()
class WOLF_ISLAND_API URepairPanel : public UUserWidget
{
    GENERATED_BODY()

public:
    // --- 데이터 및 설정 ---
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    UDataTable* ItemDataTable; // 아이콘 조회용

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    UDataTable* RepairRecipeTable; // 수리 레시피 테이블 (DT_RepairRecipes)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class URepairBlock> RepairBlockClass; // 목록에 추가할 위젯

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UCraftSlot> SlotClass; // 재료 아이콘 위젯

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    UInventoryComponent* OwnerInventory;

    // ★ 수리 대상 액터 (Spawn 시점에 넣어줘야 함)
    UPROPERTY(BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "Repair")
    ARepair_Actor* TargetRepairActor;

    // 현재 선택된 수리 데이터
    UPROPERTY(BlueprintReadOnly, Category = "Repair")
    FRepairRecipeData CurrentRepairData;
    
    // 현재 선택된 레시피의 행 이름 (데이터 테이블 수정용)
    FName CurrentRowName;


    // --- UI 바인딩 (변수 이름은 위젯 블루프린트와 일치시켜주세요) ---
    
    UPROPERTY(VisibleAnywhere, meta=(BindWidget))
    class UScrollBox* RecipeList;

    UPROPERTY(VisibleAnywhere, meta=(BindWidget))
    class UTextBlock* RecipeNameText;

    UPROPERTY(VisibleAnywhere, meta=(BindWidget))
    class UTextBlock* DescriptionText;

    UPROPERTY(VisibleAnywhere, meta=(BindWidget))
    class UTextBlock* DurationText;

    UPROPERTY(VisibleAnywhere, meta=(BindWidget))
    class UWrapBox* IngredientList;

    UPROPERTY(VisibleAnywhere, meta=(BindWidget))
    class UButton* RepairButton;


    // --- 함수들 ---

protected:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnRepairButtonClicked();

public:
    // 레시피 목록 갱신
    UFUNCTION(BlueprintCallable)
    void RefreshRecipeList();

    // 목록에 항목 하나 추가
    void AddRecipe(FName RowName, FRepairRecipeData Recipe);

    // 항목 클릭 시 상세 정보 세팅
    UFUNCTION(BlueprintCallable)
    void SetRepairInfo(FName RowName, FRepairRecipeData RecipeData);

    // 버튼 활성화/비활성화 상태 갱신
    UFUNCTION(BlueprintCallable)
    void SetRepairButtonState(FRepairRecipeData RecipeData);

    // 데이터 테이블의 Complete 값을 True로 변경하는 함수
    void MarkRecipeAsComplete(FName RowName);
};
