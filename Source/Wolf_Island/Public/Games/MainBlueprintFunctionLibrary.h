// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveInterface.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MainBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UMainBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static FString GetSavedActorGUID(const FActorSaveData& SavedActorData) { return SavedActorData.ActorID.ToString(); }
};
