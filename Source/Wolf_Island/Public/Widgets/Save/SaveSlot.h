// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveSlotPanel.h"
#include "Blueprint/UserWidget.h"
#include "SaveSlot.generated.h"

class UMainSaveGame;
class UTextBlock;
class UImage;
class UButton;
/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API USaveSlot : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(meta=(BindWidget))
	UButton* SlotButton;
	UPROPERTY(meta=(BindWidget))
	UButton* DeleteButton;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* WorldNameText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* SaveTimeText;
	UPROPERTY(meta=(BindWidget))
	UImage* WorldIcon;
	
	UMainSaveGame* SlotSave;
	
	USaveSlotPanel* SlotPanelRef;
	
	UFUNCTION(BlueprintCallable)
	void SetSlotInfo(UMainSaveGame* SaveData);
	
	UFUNCTION(BlueprintCallable)
	void SetSlotPanelRef(USaveSlotPanel* Ref) { SlotPanelRef = Ref; };
	
	UFUNCTION(BlueprintCallable)
	void OnButtonClicked();
	
	UFUNCTION(BlueprintCallable)
	void OnDeleteButtonClicked();
	
	virtual void NativeConstruct() override;
};
