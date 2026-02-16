// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/MainGameInstance.h"

#include "Games/MainSaveGame.h"
#include "Kismet/GameplayStatics.h"

void UMainGameInstance::CreateSaveSlot(FString WorldName, int32 SlotIndex, bool IsMulti)
{
	//멀티 슬롯 생성
	if (IsMulti)
	{
		UMainSaveGame* Save = Cast<UMainSaveGame>(UGameplayStatics::CreateSaveGameObject(UMainSaveGame::StaticClass()));
		Save->WorldName = WorldName;
		Save->SaveUnixTime = FDateTime::UtcNow().ToUnixTimestamp();
		UE_LOG(LogTemp, Warning, TEXT("CREATE SINGLE SAVE FILE %s [%s]"), *Save->WorldName, *FDateTime::UtcNow().ToString());
		
		FString SlotName = "M"+FString::Printf(TEXT("%03d"), SlotIndex);
		Save->SlotName = SlotName;
		UGameplayStatics::SaveGameToSlot(Save, SlotName, 0);
		UE_LOG(LogTemp, Warning, TEXT("SAVE SLOT AT [%s]"), *SlotName);
	} 
	//싱글 슬롯 생성
	else
	{
		UMainSaveGame* Save = Cast<UMainSaveGame>(UGameplayStatics::CreateSaveGameObject(UMainSaveGame::StaticClass()));
		Save->WorldName = WorldName;
		Save->SaveUnixTime = FDateTime::UtcNow().ToUnixTimestamp();
		UE_LOG(LogTemp, Warning, TEXT("CREATE SINGLE SAVE FILE %s [%s]"), *Save->WorldName, *FDateTime::UtcNow().ToString());
	
		FString SlotName = "S"+FString::Printf(TEXT("%03d"), SlotIndex);
		Save->SlotName = SlotName;
		UGameplayStatics::SaveGameToSlot(Save, SlotName, 0);
		UE_LOG(LogTemp, Warning, TEXT("SAVE SLOT AT [%s]"), *SlotName);
	}
}
