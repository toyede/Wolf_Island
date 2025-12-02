// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Data/ItemDataStruct.h"
#include "Test_ItemGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UTest_ItemGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSystem")
	TMap<FName, FSavedActorList> LevelDataMap;

	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void AddActorToLevelSave(FName LevelName, FSavedActorData NewActorData);

	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	bool GetSavedActorsFromLevel(FName LevelName, TArray<FSavedActorData>& OutActorList);

	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void ClearLevelData(FName LevelName);
	
};
