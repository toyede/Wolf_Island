// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/TextCommitPanel.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"

void UTextCommitPanel::NativeConstruct()
{
	Super::NativeConstruct();
	
	CommitButton->OnClicked.AddDynamic(this, &UTextCommitPanel::OnCommitClickedEvent);
	
	CancelButton->OnClicked.AddDynamic(this, &UTextCommitPanel::OnCancelClickedEvent);
}

void UTextCommitPanel::OnCommitClickedEvent()
{
	OnCommitClicked.Broadcast(TextEditBox->GetText().ToString());
}

void UTextCommitPanel::OnCancelClickedEvent()
{
	OnCancelClicked.Broadcast();
}
