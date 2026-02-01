// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Chatting/ChattingPanel.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Character/MainPlayerController.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Widgets/Chatting/ChattingBlock.h"

void UChattingPanel::NativeConstruct()
{
	Super::NativeConstruct();
	
	OwnedController = Cast<AMainPlayerController>(GetOwningPlayer());
	
	if (ChattingTextBox)
	{
		ChattingTextBox->OnTextCommitted.AddDynamic(this, &UChattingPanel::OnChattingCommited);
	}
}

void UChattingPanel::FocusInput()
{
	if (!ChattingTextBox) return;
	
	ChattingTextBox->SetKeyboardFocus();
}

void UChattingPanel::ClearFocusInput()
{
	if (!ChattingTextBox) return;

	//ChattingTextBox->
	UWidgetBlueprintLibrary::SetFocusToGameViewport(); 
}

void UChattingPanel::OnChattingCommited(const FText& Text, ETextCommit::Type CommitMethod)
{
	//엔터로 친 거 아니면 암것두 안함.
	if (CommitMethod != ETextCommit::OnEnter) return;
	
	//메시지 앞 뒤 공백 트림.
	const FString Msg = Text.ToString().TrimStartAndEnd();
	//빈 메시지면 암것두 안함.
	if (Msg.IsEmpty()) return;
	
	//채팅 전송 시퀀스
	if (OwnedController)
	{
		FChattingData NewChattingData = FChattingData(
			GetOwningPlayerState()->GetPlayerName(),
			Msg);
		
		OwnedController->Request_SendChat(NewChattingData);
	}
	
	//메시지 인풋 박스 비우기
	ChattingTextBox->SetText(FText::GetEmpty());
	OwnedController->ExitChatMode();
}

void UChattingPanel::AddChatting(FChattingData NewChattingData)
{
	if (!ChattingBlockClass) return;
	
	UChattingBlock* Block = CreateWidget<UChattingBlock>(this, ChattingBlockClass);
	Block->SetName(NewChattingData.Name);
	Block->SetMessage(NewChattingData.Message);
	
	ChattingBox->AddChild(Block);
	ChattingBox->ScrollToEnd();
}
