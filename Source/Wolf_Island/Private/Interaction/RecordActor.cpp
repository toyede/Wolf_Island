// Fill out your copyright notice in the Description page of Project Settings.

#include "Games/MainGameState.h"
#include "Interaction/RecordActor.h"

// Sets default values
ARecordActor::ARecordActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARecordActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARecordActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

TArray<FString> ARecordActor::GetRecordID() const
{
	TArray<FString> Options;
	Options.Add(TEXT("None"));

	UDataTable* RecordTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/item/DT_UnknownRecord.DT_UnknownRecord")));

	if (RecordTable)
	{
		TArray<FName> RowNames = RecordTable->GetRowNames();
		for (const FName& RowName : RowNames)
		{
			Options.Add(RowName.ToString());
		}
	}

	return Options;
}

void ARecordActor::Interact(AActor* Interactor)
{
	if (!HasAuthority()) return;

	if (AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>())
	{
		GS->UnlockRecord(RecordID);

		FChattingData Notice;
		Notice.Name = TEXT("알림");
		Notice.Message = TEXT("새로운 생존 기록을 발견했습니다.");
		Notice.MessageType = EMessageType::NOTICE;
		GS->AddChattingMessage(Notice);

		Destroy();
	}
}

