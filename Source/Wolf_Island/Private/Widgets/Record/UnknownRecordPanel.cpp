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
    if (RecordTable)
    {
        RecordList->ClearChildren();
        
        RecordTable->ForeachRow<FUnknownRecord>(TEXT("RecordTableContext"),
    [&](const FName& RowName, const FUnknownRecord& Record)
        {
            URecordBlock* RecordBlock = CreateWidget<URecordBlock>(GetWorld(), RecordBlockClass);

            RecordBlock->RecordName->SetText(Record.RecordTitle);
            RecordBlock->RecordData = Record;
            RecordBlock->OnRecordClicked.AddDynamic(this, &UUnknownRecordPanel::SetRecordInfo);
        
            RecordList->AddChild(RecordBlock);
        });
    }   
}

void UUnknownRecordPanel::SetRecordInfo(FUnknownRecord RecordData)
{
    RecordImage->SetBrushFromTexture(RecordData.RecordImage);
}
