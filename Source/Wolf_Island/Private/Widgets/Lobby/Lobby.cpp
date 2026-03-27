// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Lobby/Lobby.h"

#include "Character/MainPlayer.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Games/LobbyPlayerController.h"
#include "Games/MainGameInstance.h"
#include "Games/MainGameState.h"
#include "Games/MainPlayerState.h"
#include "Games/GameModes/LobbyGameMode.h"
#include "Widgets/BaseButton.h"
#include "Widgets/RoleSelection/RoleButton.h"

void ULobby::NativeConstruct()
{
	Super::NativeConstruct();
	
	PlayerController = GetOwningPlayer<ALobbyPlayerController>();
	
	if (PrevButton)
	{
		PrevButton->OnClicked.AddDynamic(this, &ULobby::OnPrev);
	}
	
	if (NextButton)
	{
		NextButton->OnClicked.AddDynamic(this, &ULobby::OnNext);
	}
	
	if (ReadyButton)
	{
		ReadyButton->OnClicked.AddDynamic(this, &ULobby::OnReady);
	}
	
	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &ULobby::OnStart);
	}
	
	SwitchPlayButton(false);
	RefreshInfo();
}

void ULobby::OnPrev()
{
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LOBBY WIDGET] NO LOBBY CONTROLLER"));
		return;
	}
	
	if (AMainPlayerState* PS = PlayerController->GetPlayerState<AMainPlayerState>())
	{
		int8 CurrentRole = static_cast<int8>(PS->GetPlayerRole());
		ECharacterRole NewRole = static_cast<ECharacterRole>(CurrentRole);
		
		if (AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>())
		{
			if (GS->GetAvailableRoles().Num() == 0) return;
			
			do {
				CurrentRole--;
				if (CurrentRole <= 0) CurrentRole = 4;
				NewRole = static_cast<ECharacterRole>(CurrentRole);
			} while (!GS->CheckAvailableRole(NewRole));
			
			//로컬에서 굳이 SetPlayerRole을 한번 더 하는 이유
			//서버에 요청하면 갔다 오는 동안 딜레이 때문에 바로바로 반영이 안됨.
			//그래서 예측성으로 로컬 값을 미리 바꿔두는 것.
			PS->SetPlayerRole(NewRole);
			PlayerController->Request_ChangeRole(NewRole);
		
			RefreshInfo();
		}
	}
}

void ULobby::OnNext()
{
	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("[LOBBY WIDGET] NO LOBBY CONTROLLER"));
		return;
	}
	
	if (AMainPlayerState* PS = PlayerController->GetPlayerState<AMainPlayerState>())
	{
		UE_LOG(LogTemp, Warning, TEXT("[LOBBY WIDGET] %s try Change Role"), *PS->GetPersistantId())
		int8 CurrentRole = static_cast<int8>(PS->GetPlayerRole());
		ECharacterRole NewRole = static_cast<ECharacterRole>(CurrentRole);
		
		if (AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>())
		{
			if (GS->GetAvailableRoles().Num() == 0) return;
			
			do {
				CurrentRole++;
				if (CurrentRole > 4) CurrentRole = 1;
				NewRole = static_cast<ECharacterRole>(CurrentRole);
			} while (!GS->CheckAvailableRole(NewRole));
		
			PS->SetPlayerRole(NewRole);
			PlayerController->Request_ChangeRole(NewRole);
		
			RefreshInfo();
		}
	}
}

void ULobby::OnReady()
{
	if (ALobbyPlayerController* LPC = Cast<ALobbyPlayerController>(PlayerController))
	{
		LPC->Request_ToggleReady();
		
		if (AMainPlayerState* PS = PlayerController->GetPlayerState<AMainPlayerState>())
		{
			SwitchRoleButton(!PS->GetIsReady());
		}
	}
}

void ULobby::OnStart()
{
	if (ALobbyGameMode* LGM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
	{
		//모두 레디 함
		if (LGM->CheckAllPlayerReady())
		{
			if (UMainGameInstance* GI = GetGameInstance<UMainGameInstance>())
			{
				UMainSaveGame* NewSave = GI->CreateSaveSlot(
					GI->CurrentServerName, 
					GI->FindEmptySaveSlotIndex(true), 
					true);
				
				GI->SetCurrentSave(NewSave);
			}
			LGM->RunGameTravel();
		}
		//어떤 쌧기가 레디 안했나
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[LOBBY WIDGET] SOMEONE DIDN'T READY"))
		}
	}
}

void ULobby::RefreshInfo()
{
	if (AMainPlayerState* PS = PlayerController->GetPlayerState<AMainPlayerState>())
	{
		ECharacterRole CurrentRole = PS->GetPlayerRole();
		
		if (RoleDataTable)
		{
			FString EnumString = StaticEnum<ECharacterRole>()->GetNameStringByValue((uint8)CurrentRole);
			FName RowName(*EnumString);
			const FRoleData* FoundRoleData = RoleDataTable->FindRow<FRoleData>(RowName, "RoleData");
		
			if (FoundRoleData)
			{
				RoleName->SetText(FoundRoleData->RoleName);
				RoleDescription->SetText(FoundRoleData->RoleDescription);
				
			}
		}
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("[LOBBY WIDGET][%hs] No Player State"), GetOwningPlayer()->HasAuthority()?"SERVER":"CLIENT");
	}
}

void ULobby::SwitchPlayButton(bool On)
{
	StartButton->Button->SetIsEnabled(On);
}

void ULobby::SwitchRoleButton(bool IsOn)
{
	NextButton->SetIsEnabled(IsOn);
	PrevButton->SetIsEnabled(IsOn);
}

