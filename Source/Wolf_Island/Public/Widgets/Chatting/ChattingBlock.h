// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Games/MainGameState.h"
#include "ChattingBlock.generated.h"

class URichTextBlock;
/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UChattingBlock : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(meta=(BindWidget))
	URichTextBlock* ChattingText;
	
public:
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable)
	void SetChattingBlock(FChattingData ChattingData);
};
