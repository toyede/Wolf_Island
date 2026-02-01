// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "ChattingBlock.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UChattingBlock : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* NameText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* MessageText;
	
public:
	
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable)
	void SetName(FString Name) { NameText->SetText(FText::FromString(Name)); };
	UFUNCTION(BlueprintCallable)
	void SetMessage(FString Message) { MessageText->SetText(FText::FromString(Message)); };	
};
