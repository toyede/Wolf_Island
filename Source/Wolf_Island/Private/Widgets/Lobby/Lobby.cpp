// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Lobby/Lobby.h"

#include "Character/MainPlayer.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Games/LobbyPlayerController.h"
#include "Games/MainPlayerState.h"
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
		CurrentRole--;
		
		if (CurrentRole <= 0) CurrentRole = 4;
		
		ECharacterRole NewRole = static_cast<ECharacterRole>(CurrentRole);
		PS->SetPlayerRole(NewRole);
		PlayerController->Request_ChangeRole(NewRole);
		
		RefreshInfo();
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
		int8 CurrentRole = static_cast<int8>(PS->GetPlayerRole());
		CurrentRole++;
		
		if (CurrentRole > 4) CurrentRole = 1;
		
		ECharacterRole NewRole = static_cast<ECharacterRole>(CurrentRole);
		PS->SetPlayerRole(NewRole);
		PlayerController->Request_ChangeRole(NewRole);
		
		RefreshInfo();
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
