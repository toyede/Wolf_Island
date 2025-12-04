// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Craft/RecipeBlock.h"
#include "UnknownRecordPanel.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UUnknownRecordPanel : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta=(BindWidget))
	class UScrollBox* RecordList;

	UPROPERTY(meta=(BindWidget))
	class UImage* RecordImage;

	UPROPERTY(EditDefaultsOnly)
	UDataTable* RecordTable;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class URecordBlock> RecordBlockClass;

	UFUNCTION()
	void RefreshList();

	UFUNCTION()
	void SetRecordInfo(FUnknownRecord RecordData);
	
protected:

	void NativeConstruct() override;
	
};
