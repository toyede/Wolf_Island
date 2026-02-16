// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveInterface.generated.h"

USTRUCT(BlueprintType)
struct FActorSaveData
{
	GENERATED_BODY()
	
	UPROPERTY()
	FGuid ActorID;
	
	UPROPERTY()
	TSubclassOf<AActor> ActorClass;

	UPROPERTY()
	FTransform Transform;
	
	UPROPERTY()
	FVector3d Velocity;

	UPROPERTY()
	TArray<uint8> BinaryData;
	
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
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
	
	virtual void SaveData(FActorSaveData& OutData0);
	virtual void LoadData(const FActorSaveData& InData);
	
	virtual FGuid GetGUID() const = 0;
};
