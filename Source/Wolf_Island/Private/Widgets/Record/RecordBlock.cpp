// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Record/RecordBlock.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"

void URecordBlock::NativeConstruct()
{
    Super::NativeConstruct();

    if (RecordButton)
    {
        RecordButton->OnClicked.AddDynamic(this, &URecordBlock::OnRecordButtonClicked);
    }
    if (RecordName)
    {
        TArray<FString> StringArray;
        RecordData.id.ParseIntoArray(StringArray, TEXT("_"), true);
        if (StringArray.Num() > 1)
        {
            RecordName->SetText(FText::FromString(StringArray[1]));
        }
        else
        {
            RecordName->SetText(FText::FromString(RecordData.title));
        } 
    }
}

void URecordBlock::OnRecordButtonClicked()
{
    OnRecordClicked.Broadcast(this, RecordData);
}

void URecordBlock::SetSelected(bool IsSelected)
{
    if (SelectedIcon)
    {
        SelectedIcon->SetVisibility(IsSelected ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    }
}


