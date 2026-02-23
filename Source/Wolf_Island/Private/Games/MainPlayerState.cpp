// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/MainPlayerState.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

void AMainPlayerState::PrintItems(float DeltaTime)
{
	for (FItemSlot& Slot : Items)
	{
		FString Item = "[ "+ Slot.ItemData.ItemName.ToString() + " | " + FString::FromInt(Slot.ItemData.Amount) + "EA ]";
		UKismetSystemLibrary::PrintString(GetWorld(), Item, true, true, FLinearColor::Green, DeltaTime);
	}	
}

void AMainPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMainPlayerState, PlayerTag);
	DOREPLIFETIME(AMainPlayerState, PlayerRole);
	DOREPLIFETIME(AMainPlayerState, Items);
}
