// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Lobby/PlayerCard.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameSession.h"
#include "Games/MainPlayerState.h"
#include "Games/GameModes/LobbyGameMode.h"

void UPlayerCard::NativeConstruct()
{
	Super::NativeConstruct();
	
	PlayerReady->SetOpacity(0.0f);
	
	if (KickButton)
	{
		if (GetWorld()->GetAuthGameMode())
		{
			KickButton->OnClicked.AddDynamic(this, &UPlayerCard::OnKickButtonClicked);
		} else
		{
			KickButton->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UPlayerCard::SetReady(bool IsReady)
{
	PlayerReady->SetOpacity(IsReady ? 1.0f : 0.0f);
}

void UPlayerCard::UpdateCard(AMainPlayerState* PlayerState)
{
	FString ID = PlayerState->GetPersistantId();
	PlayerName->SetText(FText::FromString(ID));
	SetReady(PlayerState->GetReady());
}

void UPlayerCard::OnKickButtonClicked()
{
	if (ALobbyGameMode* LGM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
	{
		LGM->GameSession->KickPlayer(PlayerController, FText::FromString("Oops"));
	}
}
