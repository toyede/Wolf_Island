// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/GameModes/MainGameMode.h"

#include "Games/MainSaveGame.h"
#include "Games/SaveInterface.h"
#include "Kismet/GameplayStatics.h"

void AMainGameMode::TestSave()
{
	UE_LOG(LogTemp, Warning, TEXT("Test Save on %hs"), HasAuthority()?"SERVER":"CLIENT");
	UMainSaveGame* Save =
		Cast<UMainSaveGame>(UGameplayStatics::CreateSaveGameObject(UMainSaveGame::StaticClass()));
	
	TArray<AActor*> SaveActors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), USaveInterface::StaticClass(), SaveActors);
	
	for (AActor* Actor : SaveActors)
	{
		if (ISaveInterface* Savable = Cast<ISaveInterface>(Actor))
		{
			FActorSaveData Data;
			Savable->SaveData(Data);
			Save->SavedActors.Add(Data);
		}
	}	
	
	UGameplayStatics::SaveGameToSlot(Save, TEXT("TestSlot"), 0);
}

void AMainGameMode::TestLoad()
{
	if (!HasAuthority()) return;
	UE_LOG(LogTemp, Warning, TEXT("Test Load on %hs"), HasAuthority()?"SERVER":"CLIENT");
	UMainSaveGame* Save =
		Cast<UMainSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("TestSlot"), 0));

	if (!Save) return;

	for (FActorSaveData& Data : Save->SavedActors)
	{
		AActor* NewActor = GetWorld()->SpawnActor<AActor>(
		Data.ActorClass,
		Data.Transform);

		if (ISaveInterface* Savable = Cast<ISaveInterface>(NewActor))
		{
			Savable->LoadData(Data);
		}
	}
}
