// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Games/GameModes/MainGameMode.h"
#include "Widgets/RoleSelection/RoleSelection.h"
#include "MultiGameMode.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API AMultiGameMode : public AMainGameMode
{
	GENERATED_BODY()
	
	bool IsMulti = true;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	
	virtual void StartingNewPlayer(APlayerController* NewPlayer) override;
	
public:
	
	//중간 합류 플레이어가 역할 선택을 마쳤을 때
	void AllocatePlayer(APlayerController* NewPlayer);
	
	//멀티에서 플레이어 죽었을 때 동작 구현.
	virtual void HandlePlayerDeath(AController* DeadPlayerController) override;
};
