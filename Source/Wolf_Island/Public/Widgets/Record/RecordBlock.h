// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemDataStruct.h"
#include "RecordBlock.generated.h"

/**
 * 
 */

class UImage;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRecordButtonClicked, URecordBlock*, ClickedBlock, FUnknownRecord, RecordData);

UCLASS()
class WOLF_ISLAND_API URecordBlock : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(BlueprintAssignable)
	FOnRecordButtonClicked OnRecordClicked;

	UPROPERTY()
	FUnknownRecord RecordData;
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UTextBlock* RecordName;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UButton* RecordButton;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UImage* SelectedIcon;

	UFUNCTION()
	void OnRecordButtonClicked();
	
	UFUNCTION()
	void SetSelected(bool IsSelected);

protected:
	virtual void NativeConstruct() override;
	
};
