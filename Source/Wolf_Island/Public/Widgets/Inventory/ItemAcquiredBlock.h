// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemAcquiredBlock.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UItemAcquiredBlock : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	class UTextBlock* InfoText;
};
