// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/MainSaveGame.h"

void UMainSaveGame::PrintSaveInfo()
{
	UE_LOG(LogTemp, Warning, TEXT("[SAVEGAME] WORLD NAME : %s"), *WorldName);
	UE_LOG(LogTemp, Warning, TEXT("[SAVEGAME] SLOT NAME : %s"), *SlotName);
	UE_LOG(LogTemp, Warning, TEXT("[SAVEGAME] NETMODE : %hs"), IsMulti?"MULTI":"SINGLE");
	
	UE_LOG(LogTemp, Warning, TEXT("[SAVEGAME] [PLAYER LIST]"));
	for (auto Player : Players)
	{
		FString PlayerID = Player.Key;
		UE_LOG(LogTemp, Warning, TEXT("[SAVEGAME] PLAYER : %s"), *PlayerID);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[SAVEGAME] [SAVED ACTOR LIST]"));
	for (auto Actor : SavedActors)
	{
		FGuid ActorID = Actor.Key;
		UE_LOG(LogTemp, Warning, TEXT("[SAVEGAME] ACTOR : %s"), *ActorID.ToString());
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[SAVEGAME] [REMOVED FOLIGE LIST]"));
	for (FRemovedFoliageData FoliageData : RemovedFoliages)
	{
		FVector FoliageLocation = FoliageData.Location;
		UE_LOG(LogTemp, Warning, TEXT("[SAVEGAME] FOLIGE LOCATION : %s"), *FoliageLocation.ToString());
	}
	
	FDateTime LocalTime = FDateTime::FromUnixTimestamp(SaveUnixTime) + (FDateTime::Now() - FDateTime::UtcNow());
	FString Time = LocalTime.ToString(TEXT("%Y년 %M월 %D일 %H:%M"));
	
	UE_LOG(LogTemp, Warning, TEXT("[SAVEGAME] SAVE TIME : %s"), *Time);
}
