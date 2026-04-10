// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MainPlayer.h"
#include "Components/StatusComponent.h"
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
	
	//플레이어 스테이트 데이터
	UPROPERTY(SaveGame)
	FString PlayerID;
	
	UPROPERTY(SaveGame)
	ECharacterRole PlayerRole;
	
	
	//액터 데이터
	UPROPERTY(SaveGame)
	FTransform Transform;
	
	UPROPERTY(SaveGame)
	FRotator ControlRotation;
	
	UPROPERTY(SaveGame)
	FVector3d Velocity;
	
	UPROPERTY(SaveGame)
	TArray<struct FItemSlot> InventoryItems;
	
	UPROPERTY(SaveGame)
	TArray<uint8> MovementBinaryData;
	
	UPROPERTY(SaveGame)
	TArray<uint8> InventoryBinaryData;
	
	UPROPERTY(SaveGame)
	TArray<uint8> StatusBinaryData;
	
	UPROPERTY(SaveGame)
	TArray<uint8> SubBinaryData1;
	
	UPROPERTY(SaveGame)
	TArray<uint8> SubBinaryData2;
	
	UPROPERTY(SaveGame)
	TArray<uint8> SubBinaryData3;

	// StatusBinaryData(컴포넌트 Serialize) 대신 구조체 기반 저장을 사용할 때 true.
	// 기존 세이브 호환을 위해 별도 플래그로 유효 여부를 구분한다.
	UPROPERTY(SaveGame)
	bool HasStatusData = false;

	UPROPERTY(SaveGame)
	FStatusSaveData StatusData;

	UPROPERTY(SaveGame)
	TArray<FName> SavedPersonalRecipes;
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
	
	UPROPERTY(SaveGame)
	FString WorldName;
	
	UPROPERTY(SaveGame)
	FString SlotName;
	
	UPROPERTY(SaveGame)
	bool IsMulti;
	
	UPROPERTY(SaveGame)
	TMap<FString, FPlayerSaveData> Players;
	
	UPROPERTY(SaveGame)
	TMap<FGuid, FActorSaveData> SavedActors;
	
	UPROPERTY(SaveGame)
	TArray<FRemovedFoliageData> RemovedFoliages;
	
	UPROPERTY(SaveGame)
	int64 SaveUnixTime;
	
	UFUNCTION(BlueprintCallable)
	void PrintSaveInfo();
	
	UFUNCTION(Blueprintable, BlueprintPure)
	FString GetSlotName() const { return SlotName; };

	UPROPERTY(BlueprintReadWrite, Category = "SaveData")
	TArray<FName> SavedSharedRecipes;

	UPROPERTY(SaveGame)
	TArray<FString> SavedUnlockedRecordIDs;
};
