// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/WolfGameUserSettings.h"

UWolfGameUserSettings* UWolfGameUserSettings::GetWolfGameUserSettings()
{
	return Cast<UWolfGameUserSettings>(UGameUserSettings::GetGameUserSettings());
}
