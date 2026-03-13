// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseButton.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBaseButtonClicked);

UCLASS()
class WOLF_ISLAND_API UBaseButton : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnBaseButtonClicked OnClicked;
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	class USizeBox* SizeBox;
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	class UBorder* BackGround;
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	class UButton* Button;
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	class UTextBlock* Text;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="1. Settings")
	FText TextContent = FText::FromString(TEXT("BUTTON"));
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="1. Settings")
	FSlateFontInfo Font;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="1. Settings")
	FMargin ButtonPadding = FMargin(0.0f, 0.0f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="1. Settings")
	FVector2D ButtonSize = FVector2D(0.0f, 0.0f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="1. Settings")
	FLinearColor ButtonColor = FLinearColor::Green;
	
	virtual void NativePreConstruct() override;
	
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OnButtonClicked();
};
