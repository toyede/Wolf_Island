// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Chatting/ChattingBlock.h"

#include "Components/RichTextBlock.h"
#include "GameFramework/PlayerState.h"

void UChattingBlock::NativeConstruct()
{
	Super::NativeConstruct();
	
}

void UChattingBlock::SetChattingBlock(FChattingData ChattingData)
{
	FDateTime LocalTime = FDateTime::FromUnixTimestamp(ChattingData.UnixTime) + (FDateTime::Now() - FDateTime::UtcNow());
	FString Time = LocalTime.ToString(TEXT("%H:%M"));
	
	FString Final;
	
	switch (ChattingData.MessageType)
	{
		case EMessageType::ALARM:
		{
			Final = FString::Printf(
			TEXT("<Time>[%s] </><Alarm>%s: </><Message>%s</>"),
			*Time,
			*ChattingData.Name,
			*ChattingData.Message);
			break;
		}
		case EMessageType::ALERT:
		{
			Final = FString::Printf(
			TEXT("<Time>[%s] </><Alert>%s: </><Message>%s</>"),
			*Time,
			*ChattingData.Name,
			*ChattingData.Message);
			break;
		}
		case EMessageType::NOTICE:
		{
			Final = FString::Printf(
			TEXT("<Time>[%s] </><Notice>%s: </><Message>%s</>"),
			*Time,
			*ChattingData.Name,
			*ChattingData.Message);
			break;
		}
		case EMessageType::GENERAL:
		{
			Final = FString::Printf(
			TEXT("<Time>[%s] </><General>%s: </><Message>%s</>"),
			*Time,
			*ChattingData.Name,
			*ChattingData.Message);
			break;
		}
	}
	
	if (ChattingData.Name == GetOwningPlayerState()->GetPlayerName())
	{
		Final = FString::Printf(
			TEXT("<Time>[%s] </><Mine>%s: </><Message>%s</>"),
			*Time,
			*ChattingData.Name,
			*ChattingData.Message);
	}
	
	ChattingText->SetText(FText::FromString(Final));
}
