// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemDataStruct.h"
#include "RepairPanel.generated.h"

class ARepair_Actor;
class UInventoryComponent;
class USoundBase;
class UTextBlock;
class UButton;
class URepairMiniGameWidget;

UCLASS()
class WOLF_ISLAND_API URepairPanel : public UUserWidget
{
    GENERATED_BODY()

public:
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    UDataTable* ItemDataTable; // 아이콘 조회용

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    UDataTable* RepairRecipeTable; // 수리 레시피 테이블

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class URepairBlock> RepairBlockClass; // 목록에 추가할 위젯

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UCraftSlot> SlotClass; // 재료 아이콘 위젯

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    UInventoryComponent* OwnerInventory;

    // 수리 대상 액터
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair")
    ARepair_Actor* TargetRepairActor;

    // 현재 선택된 수리 데이터
    UPROPERTY(BlueprintReadOnly, Category = "Repair")
    FRepairRecipeData CurrentRepairData;
    
    // 현재 선택된 레시피의 행 이름
    FName CurrentRowName;
    
    
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    TObjectPtr<USoundBase> RepairCheckSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    TObjectPtr<USoundBase> RepairSound;


protected:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnRepairButtonClicked();

    void AddSortHeader(FName SortKey);

    UFUNCTION()
    void HandleMiniGameFinished(bool bSuccess);

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
    //void MarkRecipeAsComplete(FName RowName);

    UFUNCTION(BlueprintCallable, Category = "Repair")
    void InitRepairPanel(class ARepair_Actor* InRepairActor);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    TSubclassOf<URepairMiniGameWidget> MiniGameClass;

private:
    UPROPERTY()
    URepairMiniGameWidget* ActiveMiniGame = nullptr;
};
