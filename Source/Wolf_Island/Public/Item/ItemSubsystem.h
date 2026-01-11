// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ItemSubsystem.generated.h"

struct FItemData;
/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UItemSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
	UPROPERTY()
	UDataTable* ItemDataTable;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	virtual void Deinitialize() override;
	
	FItemData* GetItemData(const FName ItemID) const;
};
