// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TextCommitPanel.generated.h"

class UEditableTextBox;
class UButton;
/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCommitClicked, const FString&, InputText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCancelClicked);

UCLASS()
class WOLF_ISLAND_API UTextCommitPanel : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(BlueprintAssignable)
	FOnCommitClicked OnCommitClicked;
	
	UPROPERTY(BlueprintAssignable)
	FOnCancelClicked OnCancelClicked;
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UEditableTextBox* TextEditBox;
	
	UPROPERTY(meta=(BindWidget))
	UButton* CommitButton;
	
	UPROPERTY(meta=(BindWidget))
	UButton* CancelButton;
	
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable)
	void OnCommitClickedEvent();
	
	UFUNCTION(BlueprintCallable)
	void OnCancelClickedEvent();
	
	UFUNCTION(BlueprintCallable)
	void OnTextCommited(const FText& Text, ETextCommit::Type CommitMethod);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetUseUppercase(bool Use);
};
