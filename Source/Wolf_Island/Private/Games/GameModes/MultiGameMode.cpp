// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/GameModes/MultiGameMode.h"

#include "GameFramework/PlayerState.h"
#include "Games/MainGameState.h"

void AMultiGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
}
