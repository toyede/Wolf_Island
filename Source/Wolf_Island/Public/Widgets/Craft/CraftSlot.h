// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CraftSlot.generated.h"

 /**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UCraftSlot : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	class UImage* ItemIcon;
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	class UTextBlock* AmountText;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UInventoryToolTip> ToolTipClass;

	UFUNCTION()
	void SetCraftSlot(const struct FItemData& ItemData, int32 Amount);

protected:

	virtual void NativeConstruct() override;
	
};
