// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Games/SaveInterface.h"
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
	TArray<struct FItemSlot> InventoryItems;
	
	UPROPERTY(SaveGame)
	TArray<uint8> InventoryBinaryData;
	
	UPROPERTY(SaveGame)
	TArray<uint8> StatusBinaryData;
};

USTRUCT(BlueprintType)
struct FRemovedFoliageData
{
	GENERATED_BODY()
	
	UPROPERTY(SaveGame)
	FVector Location;
	
	UPROPERTY(SaveGame)
	FRotator Rotation;
	
	UPROPERTY(SaveGame)
	FVector Scale;
	
	UPROPERTY(SaveGame)
	TSoftObjectPtr<UStaticMesh> Mesh;
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
	bool IsMulti;
	
	UPROPERTY()
	float CurrentTime;
	
	UPROPERTY()
	TMap<FString, FPlayerSaveData> Players;
	
	UPROPERTY()
	TMap<FGuid, FActorSaveData> SavedActors;
	
	UPROPERTY()
	TArray<FRemovedFoliageData> RemovedFoliages;
	
	UPROPERTY()
	int64 SaveUnixTime;
};
