// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Lobby/PlayerCard.h"

#include "Components/TextBlock.h"
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

void UPlayerCard::UpdateCard(AMainPlayerState* PlayerState)
{
	FString ID = PlayerState->GetPersistantId();
	PlayerName->SetText(FText::FromString(ID));
	SetReady(PlayerState->GetReady());
}
