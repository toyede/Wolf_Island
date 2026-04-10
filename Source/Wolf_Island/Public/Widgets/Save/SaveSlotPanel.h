// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SaveSlotPanel.generated.h"

class UBaseButton;
class UMainGameInstance;
class UScrollBox;
class UButton;
/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API USaveSlotPanel : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(meta=(BindWidget))
	UScrollBox* SlotBox;
	
	UPROPERTY(meta=(BindWidget))
	UBaseButton* AddSlotButton;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class USaveSlot> SlotClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UTextCommitPanel> TextCommitPanelClass;
	
	UPROPERTY()
	UTextCommitPanel* TCP;
	
	UPROPERTY(EditDefaultsOnly)
	UMainGameInstance* MainGameInstance;
	
	UPROPERTY(BlueprintReadWrite)
	bool IsMultiPanel = false;
	
	virtual void NativeOnInitialized() override;
	
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable)
	void LoadSingleSlots();
	
	UFUNCTION(BlueprintCallable)
	void LoadMultiSlots();
	
	UFUNCTION(BlueprintCallable)
	void OnAddButtonClicked();
	
	UFUNCTION(BlueprintCallable)
	void OnCreateCommited(const FString& Text);
	
	UFUNCTION(BlueprintCallable)
	void OnCancelClicked();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetIsMultiPanel() const { return IsMultiPanel; }
};
