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

	UPROPERTY(Config, BlueprintReadWrite, Category = "Input", meta = (ClampMin = "0.1", ClampMax = "3.0"))
	float MouseSensitivity = 1.0f; //JWY - 마우스 감도를 GameUserSettings에 저장해서 세팅창 Apply 이후에도 유지

	static UWolfGameUserSettings* GetWolfGameUserSettings();
};
