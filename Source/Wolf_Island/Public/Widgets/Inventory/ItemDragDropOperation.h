// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "ItemDragDropOperation.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:

	UPROPERTY()
	int32 SourceIndex;
	
	UPROPERTY()
	class UItemBase* SourceItem;

	UPROPERTY()
	class UInventoryComponent* SourceInventory;
};
