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
	
	UFUNCTION(BlueprintCallable, Category = "Repair System")
    void InitRepairWindow(ARepair_Actor* InActor);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Repair System")
	ARepair_Actor* TargetActor;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual FReply NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
		
};
