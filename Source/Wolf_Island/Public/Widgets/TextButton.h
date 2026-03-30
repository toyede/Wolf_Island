// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TextButton.generated.h"

class UTextBlock;
class UButton;
/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClickedSelf, UTextButton*, ClickedButton);

UCLASS()
class WOLF_ISLAND_API UTextButton : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(BlueprintAssignable)
	FOnClickedSelf OnClicked;
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UButton* Button;
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UTextBlock* Text;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="1. Settings")
	FText TextContent = FText::FromString(TEXT("BUTTON"));
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="1. Settings")
	FSlateFontInfo DefaultFont;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="1. Settings")
	FLinearColor DefaultTextColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="1. Settings")
	FSlateFontInfo SelectedFont;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="1. Settings")
	FLinearColor SelectedTextColor;
	
	UFUNCTION(BlueprintCallable)
	void SetSelected(bool IsSelected);
	
	UFUNCTION(BlueprintCallable)
	void OnClick();
	
	virtual void NativePreConstruct() override;
};
