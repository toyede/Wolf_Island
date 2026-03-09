// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/MainGameState.h"
#include "Character/MainPlayerController.h"
#include "Games/MainSaveGame.h"
#include "Games/GameModes/MainGameMode.h"
#include "Games/GameModes/MultiGameMode.h"
#include "Net/UnrealNetwork.h"

void AMainGameState::BeginPlay()
{
	Super::BeginPlay();
	
	UnlockedRecordIDs.Add(TEXT("REC_DIARY_01_01"));
	
	//현재 게임이 멀티인지 싱글인지
	if (HasAuthority())
	{
		IsMulti = GetWorld()->GetAuthGameMode<AMultiGameMode>() != nullptr;
	}
}

//선택된 역할 리스트 새로고침-서버에서만 새로고침 가능
void AMainGameState::RefreshSelectedRoles()
{
	if (HasAuthority())
	{
		if (AMainGameMode* GM = GetWorld()->GetAuthGameMode<AMainGameMode>())
		{
			SelectedRoles.Empty();
			
			for (auto Player : GM->PlayersSaveData)
			{
				const FPlayerSaveData& PlayerSave = Player.Value;
				SelectedRoles.Add(PlayerSave.PlayerRole);
			}
		}
	}
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
	DOREPLIFETIME(AMainGameState, SelectedRoles);
	DOREPLIFETIME(AMainGameState, IsMulti);
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

void AMainGameState::OnRep_SelectedRoles()
{
	UE_LOG(LogTemp, Warning, TEXT("SelectedRoles Updated"));
	OnSelectedRolesChanged.Broadcast();
}
