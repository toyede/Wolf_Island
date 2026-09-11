// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Lobby/PlayerCard.h"

#include "Components/TextBlock.h"
#include "GameFramework/GameSession.h"
#include "Games/MainPlayerState.h"
#include "Games/GameModes/LobbyGameMode.h"
#include "Widgets/IconButton.h"

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
	PlayerName->SetColorAndOpacity(IsReady ? ReadyNicknameColor : DefaultNicknameColor);
}

void UPlayerCard::UpdateCard(AMainPlayerState* PlayerState)
{
	FString ID = PlayerState->GetPersistantId();
	PlayerName->SetText(FText::FromString(ID));
	SetReady(PlayerState->GetReady());
	
	// 1. 현재 로컬 플레이어 및 월드 정보 가져오기
	APlayerController* LocalPC = GetOwningPlayer();
	AGameModeBase* AuthGameMode = GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr;

	// 2. 조건 확인
	// - AuthGameMode가 존재해야 서버/호스트임
	// - TargetPlayerState가 로컬 플레이어(자기 자신)의 PlayerState가 아니어야 함
	const bool bIsServer = (AuthGameMode != nullptr);
	const bool bIsSelf = (LocalPC && LocalPC->PlayerState == PlayerState);

	// 호스트이면서 '다른 플레이어'의 카드일 때만 강퇴 버튼 노출
	const bool bShouldShowKick = bIsServer && !bIsSelf;
    
	SetKickButton(bShouldShowKick);
}

void UPlayerCard::OnKickButtonClicked()
{
	if (ALobbyGameMode* LGM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
	{
		LGM->GameSession->KickPlayer(PlayerController, FText::FromString("Oops"));
	}
}

void UPlayerCard::SetKickButton(bool IsVisible)
{
	if (IsVisible)
	{
		KickButton->SetVisibility(ESlateVisibility::Visible);
	} else
	{
		KickButton->SetVisibility(ESlateVisibility::Hidden);
	}
}
