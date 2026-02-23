// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Inventory/InventoryPanel.h" 
#include "FishTrapPanel.generated.h"

UCLASS()
class WOLF_ISLAND_API UFishTrapPanel : public UInventoryPanel
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UWrapBox* FishTrapSlotBox;

	UPROPERTY(meta = (BindWidget))
	class UInventorySlot* BaitSlot;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* FishTimerText;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* BaitProgressBar;

	UPROPERTY()
	class UInventoryComponent* TrapInventoryRef;

	UPROPERTY()
	class AFishTrap* TrapRef;

	void InitializePanel(class AFishTrap* InTrap, AActor* InInteractor);
    
	UFUNCTION()
	void RefreshFishTrap();

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
};