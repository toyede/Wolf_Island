// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Record/RecordBlock.h"

#include "Components/Button.h"

void URecordBlock::NativeConstruct()
{
    UUserWidget::NativeConstruct();

    if (RecordButton)
    {
        RecordButton->OnClicked.AddDynamic(this, &URecordBlock::OnRecordButtonClicked);
    }
}

void URecordBlock::OnRecordButtonClicked()
{
    OnRecordClicked.Broadcast(RecordData);
}


