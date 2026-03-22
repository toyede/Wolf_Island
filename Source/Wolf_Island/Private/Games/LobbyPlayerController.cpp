// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/LobbyPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Games/MainPlayerState.h"
#include "Games/GameModes/LobbyGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/Lobby/Lobby.h"

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalController() && LobbyWidgetClass)
	{
		LobbyWidget = CreateWidget(this, LobbyWidgetClass);
		LobbyWidget->AddToViewport();
		
		bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(LobbyWidget->TakeWidget());
		SetInputMode(InputMode);
	}
}

void ALobbyPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ALobbyPlayerController, IsReady);
}

void ALobbyPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	if (LobbyWidget)
	{
		ULobby* Lobby = Cast<ULobby>(LobbyWidget);
		Lobby->RefreshInfo();
	}
}

void ALobbyPlayerController::Request_ToggleReady()
{
	if (HasAuthority())
	{
		ToggleReady();
	} else
	{
		Server_ToggleReady();
	}
}

void ALobbyPlayerController::Server_ToggleReady_Implementation()
{
	ToggleReady();
}

void ALobbyPlayerController::ToggleReady()
{
	IsReady = !IsReady;
}

void ALobbyPlayerController::Request_ChangeRole(ECharacterRole NewRole)
{
	if (HasAuthority())
	{
		ChangeRole(NewRole);
	} else
	{
		Server_ChangeRole(NewRole);
	}
}

void ALobbyPlayerController::Server_ChangeRole_Implementation(ECharacterRole NewRole)
{
	ChangeRole(NewRole);
}

void ALobbyPlayerController::ChangeRole(ECharacterRole NewRole)
{
	if (AMainPlayerState* PS = GetPlayerState<AMainPlayerState>())
	{
		PS->SetPlayerRole(NewRole);
		if (ALobbyGameMode* GM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
		{
			GM->RefreshSlot(this);
		}
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("[LOBBY PC] No MainPlayerState"))	
	}
}
