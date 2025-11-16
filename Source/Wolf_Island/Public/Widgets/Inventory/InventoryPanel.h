// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventorySlot.h"
#include "Blueprint/UserWidget.h"
#include "Character/MainPlayer.h"
#include "InventoryPanel.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UInventoryPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	//인벤토리 새로고침 - 인벤토리 컴포넌트에서 인벤토리 아이템 변경 시 자동 실행
	UFUNCTION(BlueprintCallable)
	void RefreshInventory();

	UPROPERTY(meta=(BindWidget))
	class UWrapBox* InventoryPanel;

	UPROPERTY(meta=(BindWidget))
	UWrapBox* HotBarPanel;
	
	UPROPERTY(meta=(BindWidget))
	class UProgressBar* WeightBar;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* WeightInfo;

	UPROPERTY()
	AMainPlayer* PlayerRef;
	
	UPROPERTY()
	UInventoryComponent* InventoryRef;
		
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UInventorySlot> SlotClass;

protected:

	UFUNCTION()
	void SetInfoText() const;

	virtual void NativeOnInitialized() override;

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
