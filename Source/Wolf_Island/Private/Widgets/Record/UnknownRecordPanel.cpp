// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Record/UnknownRecordPanel.h"
#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Data/ItemDataStruct.h"
#include "Widgets/Record/RecordBlock.h"

void UUnknownRecordPanel::NativeConstruct()
{
    UUserWidget::NativeConstruct();
    RefreshList();
}

void UUnknownRecordPanel::RefreshList()
{
    
    if (!RecordTable || !RecordBlockClass || !RecordList) return;

    RecordList->ClearChildren();
    static const FString ContextString(TEXT("Record Data Context"));
    TArray<FName> RowNames = RecordTable->GetRowNames();
    for (const FName& RowName : RowNames)
    {
        FUnknownRecord* Row = RecordTable->FindRow<FUnknownRecord>(RowName, ContextString);
        if (Row)
        {
            URecordBlock* NewBlock = CreateWidget<URecordBlock>(this, RecordBlockClass);
            if (NewBlock)
            {
                if (Row->id.IsEmpty())
                {
                    Row->id = RowName.ToString();
                }
                NewBlock->RecordData = *Row;
                NewBlock->OnRecordClicked.AddDynamic(this, &UUnknownRecordPanel::SetRecordInfo);
                RecordList->AddChild(NewBlock);
            }
        }
    }
    if (RecordList && RecordList->GetChildrenCount() > 0)
    {
        URecordBlock* FirstBlock = Cast<URecordBlock>(RecordList->GetChildAt(0));
        if (FirstBlock)
        {
            SetRecordInfo(FirstBlock->RecordData);
        }
    }
}

void UUnknownRecordPanel::SetRecordInfo(FUnknownRecord RecordData)
{
    if (RecordTitleText)
    {
        RecordTitleText->SetText(FText::FromString(RecordData.title));
    }

    if (RecordContentText)
    {
        RecordContentText->SetText(FText::FromString(RecordData.content));
    }

    if (ContentScrollBox)
    {
        ContentScrollBox->ScrollToStart();
    }
}
