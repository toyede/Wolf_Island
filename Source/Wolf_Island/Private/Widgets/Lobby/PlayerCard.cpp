// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Lobby/PlayerCard.h"

#include "Components/TextBlock.h"
#include "Games/LobbyPlayerController.h"
#include "Games/MainPlayerState.h"

void UPlayerCard::NativeConstruct()
{
	Super::NativeConstruct();
	
	PlayerReady->SetOpacity(0.0f);
}

void UPlayerCard::SetReady(bool IsReady)
{
	PlayerReady->SetOpacity(IsReady ? 1.0f : 0.0f);
}

void UPlayerCard::UpdateCard(APlayerController* PlayerController)
{
	if (ALobbyPlayerController* LPC = Cast<ALobbyPlayerController>(PlayerController))
	{
		FString ID = LPC->GetPlayerState<AMainPlayerState>()->GetPersistantId();
		PlayerName->SetText(FText::FromString(ID));
		SetReady(LPC->IsReady);
	}
}
