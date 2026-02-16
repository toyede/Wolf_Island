// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveInterface.h"
#include "Data/ItemDataStruct.h"
#include "GameFramework/SaveGame.h"
#include "MainSaveGame.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FPlayerSaveData
{
	GENERATED_BODY()
	
	UPROPERTY(SaveGame)
	FString PlayerID;
	
	UPROPERTY(SaveGame)
	FTransform Transform;
	
	UPROPERTY(SaveGame)
	FRotator ControlRotation;
	
	UPROPERTY(SaveGame)
	FVector3d Velocity;
	
	UPROPERTY(SaveGame)
	TArray<FItemSlot> InventoryItems;
	
	UPROPERTY(SaveGame)
	TArray<uint8> InventoryBinaryData;
	
	UPROPERTY(SaveGame)
	TArray<uint8> StatusBinaryData;
};

UCLASS()
class WOLF_ISLAND_API UMainSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY()
	FString WorldName;
	
	UPROPERTY()
	FString SlotName;
	
	UPROPERTY()
	TMap<FString, FPlayerSaveData> Players;
	
	UPROPERTY()
	TArray<FActorSaveData> SavedActors;
	
	UPROPERTY()
	int64 SaveUnixTime;
};
