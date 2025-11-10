// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/InventoryComponent.h"
#include "PlayerHUD.generated.h"

/**
 * 
 */

class UProgressBar;
class UItemAcquiredBlock;

UCLASS()
class WOLF_ISLAND_API UPlayerHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable)
	void AddItemMessage(FItemAddResult Result);

	UPROPERTY(EditAnywhere)
	class AMainPlayer* PlayerRef;
		
	UPROPERTY(meta=(BindWidget))
	UProgressBar* InteractionBar;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UItemAcquiredBlock> ItemAcquiredBlockClass;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UVerticalBox* InfoList;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UWrapBox* HotBar;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UInventorySlot> SlotClass;

	UPROPERTY()
	bool ShowInteraction = true;

	UFUNCTION(BlueprintCallable)
	void DisplayInteraction();
	UFUNCTION(BlueprintCallable)
	void HideInteraction();
	UFUNCTION(BlueprintCallable)
	void ToggleInteraction();
	UFUNCTION(BlueprintCallable)
	void UpdateInteraction();

	UFUNCTION(BlueprintCallable)
	void RefreshHotBar();


protected:
	
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
};
