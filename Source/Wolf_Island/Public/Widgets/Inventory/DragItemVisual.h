// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DragItemVisual.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UDragItemVisual : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	class UBorder* ItemBorder;

	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	class UImage* ItemIcon;

	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	class UTextBlock* ItemAmount;
};
