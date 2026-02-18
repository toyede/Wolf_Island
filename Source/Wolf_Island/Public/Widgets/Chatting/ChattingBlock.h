// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Games/MainGameState.h"
#include "ChattingBlock.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UChattingBlock : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* TimeText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* NameText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* MessageText;
	
public:
	
	UPROPERTY(EditDefaultsOnly, Category="Color")
	FSlateColor GeneralColor;

	UPROPERTY(EditDefaultsOnly, Category="Color")
	FSlateColor AlarmColor;

	UPROPERTY(EditDefaultsOnly, Category="Color")
	FSlateColor AlertColor;
	
	UPROPERTY(EditDefaultsOnly, Category="Color")
	FSlateColor NoticeColor;
	
	UPROPERTY(EditDefaultsOnly, Category="Color")
	FSlateColor OwnerColor;
	
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable)
	void SetChattingBlock(FChattingData ChattingData);
	
	UFUNCTION(BlueprintCallable)
	void SetTime(int32 UnixTime) 
	{ 
		FDateTime LocalTime = FDateTime::FromUnixTimestamp(UnixTime) + (FDateTime::Now() - FDateTime::UtcNow());
		TimeText->SetText(FText::FromString(LocalTime.ToString(TEXT("[%H:%M]")))); 
	};
	UFUNCTION(BlueprintCallable)
	void SetName(FString Name) { NameText->SetText(FText::FromString(Name)); };
	UFUNCTION(BlueprintCallable)
	void SetMessage(FString Message) { MessageText->SetText(FText::FromString(Message)); };	
};
