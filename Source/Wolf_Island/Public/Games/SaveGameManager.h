// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveGameManager.generated.h"

/**
 * 
 */



UCLASS()
class WOLF_ISLAND_API USaveGameManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
	//저장할 것들
	//플레이어 정보의 목록(멀티 플레이)
	//월드의 액터들
	//월드 진행 정보
	
};
