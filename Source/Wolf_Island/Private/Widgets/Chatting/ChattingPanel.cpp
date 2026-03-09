// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Chatting/ChattingPanel.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Character/MainPlayerController.h"
#include "Components/Border.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Widgets/Chatting/ChattingBlock.h"

void UChattingPanel::NativeConstruct()
{
	Super::NativeConstruct();
	
	OwnedController = Cast<AMainPlayerController>(GetOwningPlayer());
	
	if (ChattingInputBox)
	{
		ChattingInputBox->OnTextCommitted.AddDynamic(this, &UChattingPanel::OnChattingCommited);
	}
	
	if (ChattingList)
	{
		ChattingList->ClearChildren();
	}
}

void UChattingPanel::OnOpenAnimationFinished()
{
	ChattingInputBox->SetKeyboardFocus();
}

void UChattingPanel::OnCloseAnimationFinished()
{
	ChattingList->ScrollToEnd();
}

void UChattingPanel::FocusInput()
{
	if (!ChattingInputBox) return;
	
	FWidgetAnimationDynamicEvent EndEvent;
	EndEvent.BindDynamic(this, &UChattingPanel::OnOpenAnimationFinished);

	BindToAnimationFinished(OpenAnim, EndEvent);
	
	PlayAnimation(OpenAnim);
}

void UChattingPanel::ClearFocusInput()
{
	if (!ChattingInputBox) return;
	
	FWidgetAnimationDynamicEvent EndEvent;
	EndEvent.BindDynamic(this, &UChattingPanel::OnCloseAnimationFinished);
	
	BindToAnimationFinished(CloseAnim, EndEvent);
	
	PlayAnimation(CloseAnim);
	UWidgetBlueprintLibrary::SetFocusToGameViewport(); 
}

void UChattingPanel::OnChattingCommited(const FText& Text, ETextCommit::Type CommitMethod)
{
	//마우스로 아무데나 클릭하면 채팅모드 나가기
	if (CommitMethod == ETextCommit::OnCleared || CommitMethod == ETextCommit::OnUserMovedFocus)
	{
		OwnedController->ExitChatMode();
		return;
	}
	
	//엔터로 친 거 아니면 암것두 안함.
	if (CommitMethod != ETextCommit::OnEnter) return;
	
	//메시지 앞 뒤 공백 트림.
	const FString Msg = Text.ToString().TrimStartAndEnd();
	//빈 메시지면 암것두 안함.
	if (Msg.IsEmpty())
	{
		OwnedController->ExitChatMode();
		return;
	}
	
	//채팅 전송 시퀀스
	if (OwnedController)
	{
		FChattingData NewChattingData = FChattingData(
			GetOwningPlayerState()->GetPlayerName(),
			Msg);
		
		OwnedController->Request_SendChat(NewChattingData);
	}
	
	//메시지 인풋 박스 비우기
	ChattingInputBox->SetText(FText::GetEmpty());
	OwnedController->ExitChatMode();
}

void UChattingPanel::AddChatting(FChattingData NewChattingData)
{
	if (!ChattingBlockClass) return;
	
	UChattingBlock* Block = CreateWidget<UChattingBlock>(this, ChattingBlockClass);
	Block->SetChattingBlock(NewChattingData);
	
	ChattingList->InsertChildAt(0, Block);
	ForceLayoutPrepass();
	ChattingList->ScrollToEnd();
}
