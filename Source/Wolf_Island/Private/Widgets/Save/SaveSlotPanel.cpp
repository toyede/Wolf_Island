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
}

void USaveSlotPanel::LoadSingleSlots()
{
	IsMultiPanel = false;
	SlotBox->ClearChildren();
	
	FString Prefix = "S";
	
	for (int SlotIndex=0; SlotIndex<MaxSlotIndex; SlotIndex++)
	{
		FString SlotName = Prefix+FString::Printf(TEXT("%03d"), SlotIndex);
		UMainSaveGame* Save =  Cast<UMainSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
		
		if (Save && SlotClass)
		{
			USaveSlot* SaveSlot = CreateWidget<USaveSlot>(GetWorld(), SlotClass);
			SaveSlot->SetSlotInfo(Save);
			SaveSlot->SetSlotPanelRef(this);
			SlotBox->AddChild(SaveSlot);
		}
	}
	if (SlotBox->GetChildrenCount() >= MaxSlotIndex) return;
	SlotBox->AddChild(AddSlotButton);	
}

void USaveSlotPanel::LoadMultiSlots()
{
	IsMultiPanel = true;
	SlotBox->ClearChildren();
	
	FString Prefix = "M";
	
	for (int SlotIndex=0; SlotIndex<MaxSlotIndex; SlotIndex++)
	{
		FString SlotName = Prefix+FString::Printf(TEXT("%03d"), SlotIndex);
		UMainSaveGame* Save =  Cast<UMainSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
		
		if (Save && SlotClass)
		{
			USaveSlot* SaveSlot = CreateWidget<USaveSlot>(GetWorld(), SlotClass);
			SaveSlot->SetSlotInfo(Save);
			SaveSlot->SetSlotPanelRef(this);
			SlotBox->AddChild(SaveSlot);
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Loading MultiSlots : %d"), SlotBox->GetChildrenCount());
	if (SlotBox->GetChildrenCount() >= MaxSlotIndex) return;
	SlotBox->AddChild(AddSlotButton);
}

void USaveSlotPanel::OnAddButtonClicked()
{
	UTextCommitPanel* TextCommitPanel = CreateWidget<UTextCommitPanel>(GetWorld(), TextCommitPanelClass);
	TCP = TextCommitPanel;
	TCP->OnCommitClicked.AddDynamic(this, &USaveSlotPanel::OnCreateCommited);
	TCP->AddToViewport();
}

void USaveSlotPanel::OnCreateCommited(const FString& Text)
{
	UMainGameInstance* GS = Cast<UMainGameInstance>(GetGameInstance());
	GS->CreateSaveSlot(Text, SlotBox->GetChildrenCount()-1, IsMultiPanel);
	TCP->RemoveFromParent();
	RemoveFromParent();
}
