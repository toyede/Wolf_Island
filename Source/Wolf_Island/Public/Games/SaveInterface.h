// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveInterface.generated.h"

USTRUCT(BlueprintType)
struct FActorSaveData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, SaveGame)
	FGuid ActorID;
	
	UPROPERTY(BlueprintReadWrite, SaveGame)
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	FTransform Transform;
	
	UPROPERTY(BlueprintReadWrite, SaveGame)
	FVector Velocity;

	UPROPERTY(BlueprintReadWrite, SaveGame)
	TArray<uint8> BinaryData;
	
	UPROPERTY(BlueprintReadWrite, SaveGame)
	TArray<uint8> SubBinaryData1;
	
	UPROPERTY(BlueprintReadWrite, SaveGame)
	TArray<uint8> SubBinaryData2;
	
	UPROPERTY(BlueprintReadWrite, SaveGame)
	TArray<uint8> SubBinaryData3;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class USaveInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class WOLF_ISLAND_API ISaveInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SaveData(FActorSaveData& OutData);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void LoadData(const FActorSaveData& InData);
	
	virtual FGuid GetGUID() const = 0;
};
