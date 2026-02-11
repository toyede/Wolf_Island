// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Chatting/ChattingBlock.h"

#include "GameFramework/PlayerState.h"

void UChattingBlock::NativeConstruct()
{
	Super::NativeConstruct();
	
}

void UChattingBlock::SetChattingBlock(FChattingData ChattingData)
{
	SetTime(ChattingData.UnixTime);
	SetName(ChattingData.Name);
	SetMessage(ChattingData.Message);

	switch (ChattingData.MessageType)
	{
		case EMessageType::ALARM:
		{
			NameText->SetColorAndOpacity(AlarmColor);
			break;
		}
		case EMessageType::ALERT:
		{
			NameText->SetColorAndOpacity(AlertColor);
			break;
		}
		case EMessageType::NOTICE:
		{
			NameText->SetColorAndOpacity(NoticeColor);
			break;
		}
		case EMessageType::GENERAL:
		{
			NameText->SetColorAndOpacity(GeneralColor);
			break;
		}
	}
	
	if (ChattingData.Name == GetOwningPlayerState()->GetPlayerName())
	{
		NameText->SetColorAndOpacity(OwnerColor);
	}
}
