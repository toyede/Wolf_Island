// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/MainGameInstance.h"

#include "Games/MainSaveGame.h"
#include "Kismet/GameplayStatics.h"

UMainSaveGame* UMainGameInstance::CreateSaveSlot(FString WorldName, int32 SlotIndex, bool IsMulti)
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
		Save->IsMulti = IsMulti;
		UGameplayStatics::SaveGameToSlot(Save, SlotName, 0);
		UE_LOG(LogTemp, Warning, TEXT("SAVE SLOT AT [%s]"), *SlotName);
		return Save;
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
		Save->IsMulti = IsMulti;
		UGameplayStatics::SaveGameToSlot(Save, SlotName, 0);
		UE_LOG(LogTemp, Warning, TEXT("SAVE SLOT AT [%s]"), *SlotName);
		return Save;
	}
}

int32 UMainGameInstance::FindEmptySaveSlotIndex(bool IsMulti)
{
	FString Prefix = IsMulti ? TEXT("M") : TEXT("S");
	
	for (int32 i = 0; i < MaxSlotIndex; i++)
	{
		FString SlotName = Prefix + FString::Printf(TEXT("%03d"), i);

		if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
		{
			return i;
		}
	}
	
	return -1;
}

void UMainGameInstance::Init()
{
	Super::Init();
	
	
}


