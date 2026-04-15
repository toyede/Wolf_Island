// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/TextCommitPanel.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"

void UTextCommitPanel::NativeConstruct()
{
	Super::NativeConstruct();
	
	UE_LOG(LogTemp, Warning, TEXT("[TCP] TPC CREATED"));
	
	CommitButton->OnClicked.AddDynamic(this, &UTextCommitPanel::OnCommitClickedEvent);
	
	CancelButton->OnClicked.AddDynamic(this, &UTextCommitPanel::OnCancelClickedEvent);
	
	TextEditBox->OnTextCommitted.AddDynamic(this, &UTextCommitPanel::OnTextCommited);
}

void UTextCommitPanel::OnCommitClickedEvent()
{
	CommitButton->SetIsEnabled(false);
	OnCommitClicked.Broadcast(TextEditBox->GetText().ToString());
}

void UTextCommitPanel::OnCancelClickedEvent()
{
	OnCancelClicked.Broadcast();
}

void UTextCommitPanel::OnTextCommited(const FText& Text, ETextCommit::Type CommitMethod)
{
	//엔터로 친 거 아니면 암것두 안함.
	if (CommitMethod != ETextCommit::OnEnter) return;
	
	OnCommitClicked.Broadcast(TextEditBox->GetText().ToString());
}

void UTextCommitPanel::SetUseUppercase_Implementation(bool Use)
{
}
