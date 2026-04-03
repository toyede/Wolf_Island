// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Save/SaveSlot.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Games/MainGameInstance.h"
#include "Games/MainSaveGame.h"
#include "Kismet/GameplayStatics.h"

void USaveSlot::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (SlotButton)
	{
		SlotButton->OnClicked.AddDynamic(this, &USaveSlot::OnButtonClicked);
	}
	
	if (DeleteButton)
	{
		DeleteButton->OnClicked.AddDynamic(this, &USaveSlot::OnDeleteButtonClicked);
	}
	
	MainGameInstance = Cast<UMainGameInstance>(GetGameInstance());
}

void USaveSlot::SetSlotInfo(UMainSaveGame* SaveData)
{
	WorldNameText->SetText(FText::FromString(SaveData->WorldName));
	
	FDateTime DateTime = FDateTime::FromUnixTimestamp(SaveData->SaveUnixTime);
	FText LocalizedTime = FText::AsDateTime(DateTime);
	SaveTimeText->SetText(LocalizedTime);
	
	SlotSave = SaveData;
}

void USaveSlot::OnButtonClicked()
{
	MainGameInstance->SetCurrentSave(SlotSave);
	
	//멀티 게임 시작
	if (SlotSave->IsMulti)
	{
		MainGameInstance->CreateGameSession();
	}
	//싱글 게임 시작
	else
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), MainGameInstance->SinglePlayWorld);
	}
}

void USaveSlot::OnDeleteButtonClicked()
{
	UGameplayStatics::DeleteGameInSlot(SlotSave->SlotName, 0);
	if (SlotPanelRef->IsMultiPanel)
	{
		SlotPanelRef->LoadMultiSlots();
	} else
	{
		SlotPanelRef->LoadSingleSlots();
	}
}

