// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Data/ItemDataStruct.h"
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
	UItemBase* SourceItem;
	
	UPROPERTY()
	FItemBaseData SourceItemData;

	UPROPERTY()
	class UInventoryComponent* SourceInventory;

	//우클릭 반갈처럼 드래그 시작 시 소스에서 미리 개수를 뺀 경우 true.
	//드래그가 취소되면 SourceItemData.Amount만큼 소스에 되돌려야 함.
	UPROPERTY()
	bool bWasSplit = false;
};
