// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/MainGameState.h"

#include "Character/MainPlayerController.h"
#include "Net/UnrealNetwork.h"

void AMainGameState::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMainGameState::AddChattingMessage(FChattingData NewChattingData)
{
	ChattingData.Add(NewChattingData);
	Multi_AddChat(NewChattingData);
}

FChattingData AMainGameState::GetLastChattingData()
{
	FChattingData LastChattingData = FChattingData();
	
	if (ChattingData.Num() == 0) return LastChattingData;
	
	LastChattingData = ChattingData.Last();
	
	return LastChattingData;
}


void AMainGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMainGameState, ChattingData);
}

void AMainGameState::Multi_AddChat_Implementation(FChattingData NewChattingData)
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (AMainPlayerController* PlayerController = Cast<AMainPlayerController>(PC))
		{
			PlayerController->AddChat(NewChattingData);
		}
	}
}
