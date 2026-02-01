// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Chatting/ChattingBlock.h"

#include "GameFramework/PlayerState.h"

void UChattingBlock::NativeConstruct()
{
	Super::NativeConstruct();
	
}

void UChattingBlock::SetChattingBlock(FChattingData ChattingData)
{
	SetName(ChattingData.Name);
	SetMessage(ChattingData.Message);

	switch (ChattingData.MessageType)
	{
		case EMessageType::ALARM:
		{
			NameText->SetColorAndOpacity(AlarmColor);
		}
		case EMessageType::ALERT:
		{
			NameText->SetColorAndOpacity(AlertColor);
		}
		case EMessageType::NOTICE:
		{
			NameText->SetColorAndOpacity(NoticeColor);
		}
		case EMessageType::GENERAL:
		{
			NameText->SetColorAndOpacity(GeneralColor);
		}
	}
	
	if (ChattingData.Name == GetOwningPlayerState()->GetPlayerName())
	{
		NameText->SetColorAndOpacity(OwnerColor);
	}
}
