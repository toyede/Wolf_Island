// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/MainGameState.h"
#include "Character/MainPlayerController.h"
#include "Games/MainGameState.h"
#include "Character/MainPlayerController.h"
#include "Net/UnrealNetwork.h"

void AMainGameState::BeginPlay()
{
	Super::BeginPlay();
	UnlockedRecordIDs.Add(TEXT("REC_DIARY_01_01"));
}

void AMainGameState::AddChattingMessage(FChattingData NewChattingData)
{
	ChattingData.Add(NewChattingData);
	Multi_AddChat(NewChattingData);
}

FChattingData AMainGameState::GetLastChattingData()
{
	FChattingData LastChattingData = FChattingData();
	
	if (ChattingData.Num() == 0) return LastChattingData;
	
	LastChattingData = ChattingData.Last();
	
	return LastChattingData;
}


void AMainGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMainGameState, ChattingData);
	DOREPLIFETIME(AMainGameState, UnlockedRecordIDs);
}

void AMainGameState::Multi_AddChat_Implementation(FChattingData NewChattingData)
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AMainPlayerController* PlayerController = Cast<AMainPlayerController>(PC))
		{
			PlayerController->AddChat(NewChattingData);
		}
	}
}

void AMainGameState::UnlockRecord(const FString& RecordID)
{
	if (HasAuthority())
	{
		if (!RecordID.IsEmpty() && !UnlockedRecordIDs.Contains(RecordID))
		{
			UnlockedRecordIDs.Add(RecordID);
            
			OnUnlockedRecordsChanged.Broadcast();
		}
	}
}

void AMainGameState::OnRep_UnlockedRecordIDs()
{
	OnUnlockedRecordsChanged.Broadcast();
}
