// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemDataStruct.h"
#include "RepairUI.generated.h"

/**
 * 
 */
class ARepair_Actor;
 
UCLASS()
class WOLF_ISLAND_API URepairUI : public UUserWidget
{
	GENERATED_BODY()
    
public:
	// WBP의 CraftPanel 위젯
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	class URepairPanel* WBP_RepairPanel;

	// WBP의 닫기 버튼 (이름을 CloseButton으로 맞춰주세요)
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	class UButton* CloseButton;
	
	UFUNCTION(BlueprintCallable, Category = "Repair System")
    void InitRepairWindow(ARepair_Actor* InActor);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Repair System")
	ARepair_Actor* TargetActor;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// [추가] 키보드 입력을 처리하는 함수 (탭 키 감지용)
	virtual FReply NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	UFUNCTION()
	void HandleCloseClicked();
	
};
