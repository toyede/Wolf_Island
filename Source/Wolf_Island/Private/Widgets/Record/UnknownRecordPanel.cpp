// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Record/UnknownRecordPanel.h"
#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Data/ItemDataStruct.h"
#include "Games/MainGameState.h"
#include "Widgets/Record/RecordBlock.h"

void UUnknownRecordPanel::NativeConstruct()
{
    UUserWidget::NativeConstruct();
    if (AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>())
    {
        GS->OnUnlockedRecordsChanged.AddDynamic(this, &UUnknownRecordPanel::RefreshList);
    }
    
    RefreshList();
}

void UUnknownRecordPanel::RefreshList()
{
    if (!RecordTable || !RecordBlockClass || !RecordList) return;

    RecordList->ClearChildren();

    AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>();
    if (!GS) return;

    static const FString ContextString(TEXT("Record Data Context"));
    TArray<FName> RowNames = RecordTable->GetRowNames();

    for (const FName& RowName : RowNames)
    {
        FUnknownRecord* Row = RecordTable->FindRow<FUnknownRecord>(RowName, ContextString);
        if (Row)
        {
            FString CheckID = Row->id.IsEmpty() ? RowName.ToString() : Row->id;

            if (GS->UnlockedRecordIDs.Contains(CheckID))
            {
                URecordBlock* NewBlock = CreateWidget<URecordBlock>(this, RecordBlockClass);
                if (NewBlock)
                {
                    NewBlock->RecordData = *Row;
                    if (NewBlock->RecordData.id.IsEmpty()) NewBlock->RecordData.id = RowName.ToString();

                    NewBlock->OnRecordClicked.AddDynamic(this, &UUnknownRecordPanel::SetRecordInfo);
                    RecordList->AddChild(NewBlock);
                }
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
