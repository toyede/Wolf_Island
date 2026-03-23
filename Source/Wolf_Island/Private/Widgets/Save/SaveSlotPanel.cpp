// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Save/SaveSlotPanel.h"

#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Games/MainGameInstance.h"
#include "Games/MainSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Save/SaveSlot.h"
#include "Widgets/TextCommitPanel.h"

//귀찮으니 최대 슬롯은 5개로 제한함.

void USaveSlotPanel::NativeConstruct()
{
	Super::NativeConstruct();
	
	AddSlotButton->OnClicked.AddDynamic(this, &USaveSlotPanel::OnAddButtonClicked);
	
	MainGameInstance = Cast<UMainGameInstance>(GetGameInstance());
}

void USaveSlotPanel::LoadSingleSlots()
{
	IsMultiPanel = false;
	SlotBox->ClearChildren();

	FString Prefix = "S";

	//Save 데이터 모으기
	TArray<TPair<FString, UMainSaveGame*>> LoadedSaves;
	int32 MaxSlotIndex = MainGameInstance->GetMaxSlotIndex();
	UE_LOG(LogTemp, Warning, TEXT("MAX INDEX %d"), MaxSlotIndex);

	for (int SlotIndex = 0; SlotIndex < MaxSlotIndex; SlotIndex++)
	{
		FString SlotName = Prefix + FString::Printf(TEXT("%03d"), SlotIndex);
		UMainSaveGame* Save = Cast<UMainSaveGame>(
			UGameplayStatics::LoadGameFromSlot(SlotName, 0)
		);

		if (Save)
		{
			LoadedSaves.Add(TPair<FString, UMainSaveGame*>(SlotName, Save));
		}
	}

	//최신순 정렬 (내림차순)
	LoadedSaves.Sort([](const TPair<FString, UMainSaveGame*>& A,
						const TPair<FString, UMainSaveGame*>& B)
	{
		return A.Value->SaveUnixTime > B.Value->SaveUnixTime;
	});

	//정렬된 순서대로 위젯 생성
	for (const TPair<FString, UMainSaveGame*>& Pair : LoadedSaves)
	{
		if (SlotClass)
		{
			USaveSlot* SaveSlot = CreateWidget<USaveSlot>(GetWorld(), SlotClass);
			SaveSlot->SetSlotInfo(Pair.Value);
			SaveSlot->SetSlotPanelRef(this);
			SaveSlot->SetPadding(FMargin(0.0, 0.0, 0.0, 16.0));

			SlotBox->AddChild(SaveSlot);
		}
	}

	//빈 슬롯이 남아있으면 Add 버튼 추가
	if (SlotBox->GetChildrenCount() < MaxSlotIndex)
	{
		if (AddSlotButton)
		{
			SlotBox->AddChild(AddSlotButton);
		}
	}
}

void USaveSlotPanel::LoadMultiSlots()
{
	IsMultiPanel = true;
	SlotBox->ClearChildren();

	FString Prefix = "M";

	//Save 데이터 모으기
	TArray<TPair<FString, UMainSaveGame*>> LoadedSaves;
	int32 MaxSlotIndex = MainGameInstance->GetMaxSlotIndex();
	UE_LOG(LogTemp, Warning, TEXT("MAX INDEX %d"), MaxSlotIndex);

	for (int SlotIndex = 0; SlotIndex < MaxSlotIndex; SlotIndex++)
	{
		FString SlotName = Prefix + FString::Printf(TEXT("%03d"), SlotIndex);
		UMainSaveGame* Save = Cast<UMainSaveGame>(
			UGameplayStatics::LoadGameFromSlot(SlotName, 0)
		);

		if (Save)
		{
			LoadedSaves.Add(TPair<FString, UMainSaveGame*>(SlotName, Save));
		}
	}

	//최신순 정렬 (내림차순)
	LoadedSaves.Sort([](const TPair<FString, UMainSaveGame*>& A,
						const TPair<FString, UMainSaveGame*>& B)
	{
		return A.Value->SaveUnixTime > B.Value->SaveUnixTime;
	});

	//정렬된 순서대로 위젯 생성
	for (const TPair<FString, UMainSaveGame*>& Pair : LoadedSaves)
	{
		if (SlotClass)
		{
			USaveSlot* SaveSlot = CreateWidget<USaveSlot>(GetWorld(), SlotClass);
			SaveSlot->SetSlotInfo(Pair.Value);
			SaveSlot->SetSlotPanelRef(this);
			SaveSlot->SetPadding(FMargin(0.0, 0.0, 0.0, 16.0));

			SlotBox->AddChild(SaveSlot);
		}
	}

	//빈 슬롯이 남아있으면 Add 버튼 추가
	if (SlotBox->GetChildrenCount() < MaxSlotIndex)
	{
		if (AddSlotButton)
		{
			SlotBox->AddChild(AddSlotButton);
		}
	}
}

void USaveSlotPanel::OnAddButtonClicked()
{
	UTextCommitPanel* TextCommitPanel = CreateWidget<UTextCommitPanel>(GetWorld(), TextCommitPanelClass);
	TCP = TextCommitPanel;
	TCP->OnCommitClicked.AddDynamic(this, &USaveSlotPanel::OnCreateCommited);
	TCP->OnCancelClicked.AddDynamic(this, &USaveSlotPanel::OnCancelClicked);
	TCP->AddToViewport();
}

void USaveSlotPanel::OnCreateCommited(const FString& Text)
{
	if (MainGameInstance)
	{
		UMainSaveGame* NewSave = MainGameInstance->CreateSaveSlot(Text, MainGameInstance->FindEmptySaveSlotIndex(IsMultiPanel), IsMultiPanel);
		IsMultiPanel ? LoadMultiSlots() : LoadSingleSlots();
		TCP->RemoveFromParent();
		
		MainGameInstance->SetCurrentSave(NewSave);
	
		//멀티 게임 시작
		if (NewSave->IsMulti)
		{
			MainGameInstance->CreateSession();
			//UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), MainGameInstance->MultiLobbyWorld);
		}
		//싱글 게임 시작
		else
		{
			UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), MainGameInstance->SinglePlayWorld);
		}
	}
}

void USaveSlotPanel::OnCancelClicked()
{
	if (TCP)
	{
		TCP->RemoveFromParent();
	}
}
