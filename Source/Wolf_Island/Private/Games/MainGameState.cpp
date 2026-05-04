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
	
	if (IsMulti)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAMESTATE] This Game is MULTI"));
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAMESTATE] This Game is SINGLE"));
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
			
			for (auto PS : PlayerArray)
			{
				if (AMainPlayerState* MPS = Cast<AMainPlayerState>(PS))
				{
					SelectedRoles.Add(MPS->GetPlayerRole());
				}
			}
			
			OnSelectedRolesChanged.Broadcast();
		}
	}
}

bool AMainGameState::CheckAvailableRole(ECharacterRole NewRole)
{
	for ( auto PS : PlayerArray)
	{
		if (AMainPlayerState* MPS = Cast<AMainPlayerState>(PS))
		{
			UE_LOG(LogTemp, Warning, TEXT("[GAMSTATE] %d is Available"), MPS->GetPlayerRole());
			if (MPS->GetPlayerRole() == NewRole) return false;
		}
	}
	
	return true;
}

TArray<ECharacterRole> AMainGameState::GetAvailableRoles()
{
	TArray<ECharacterRole> AvailableRoles;
	AvailableRoles.Add(ECharacterRole::CAPTAIN);
	AvailableRoles.Add(ECharacterRole::CHEF);
	AvailableRoles.Add(ECharacterRole::MECHANIC);
	AvailableRoles.Add(ECharacterRole::SOLDIER);
	
	for ( auto PS : PlayerArray )
	{
		if (AMainPlayerState* MPS = Cast<AMainPlayerState>(PS))
		{
			AvailableRoles.Remove(MPS->GetPlayerRole());
		}
	}	
	
	return AvailableRoles;
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
	DOREPLIFETIME(AMainGameState, SharedRecipes);
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

void AMainGameState::UnlockSharedRecipe(const FName& RecipeID)
{
	if (HasAuthority() && !RecipeID.IsNone() && !SharedRecipes.Contains(RecipeID))
	{
		SharedRecipes.Add(RecipeID);
		OnSharedRecipesChanged.Broadcast();
	}
}

void AMainGameState::OnRep_SharedRecipes()
{
	OnSharedRecipesChanged.Broadcast();
}
