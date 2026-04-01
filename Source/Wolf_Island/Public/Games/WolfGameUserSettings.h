// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "WolfGameUserSettings.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UWolfGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, BlueprintReadWrite, Category = "Audio")
	float MasterVolume = 1.0f;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Audio")
	float BGMVolume = 1.0f;

	UPROPERTY(Config, BlueprintReadWrite, Category = "Audio")
	float SFXVolume = 1.0f;

	static UWolfGameUserSettings* GetWolfGameUserSettings();
};
