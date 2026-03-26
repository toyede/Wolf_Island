// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/MainPlayerState.h"

#include "GameFramework/GameStateBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

AMainPlayerState::AMainPlayerState()
{
	bActorSeamlessTraveled = true;
}

void AMainPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	
	AMainPlayerState* NewPS = Cast<AMainPlayerState>(PlayerState);
	if (NewPS)
	{
		NewPS->PlayerRole = PlayerRole;
		NewPS->IsReady = IsReady;
		NewPS->Items = Items;
	}
}

void AMainPlayerState::OverrideWith(APlayerState* PlayerState)
{
	Super::OverrideWith(PlayerState);
	
	AMainPlayerState* OldPS = Cast<AMainPlayerState>(PlayerState);
	if (OldPS)
	{
		PlayerRole = OldPS->PlayerRole;
		IsReady = OldPS->IsReady;
		Items = OldPS->Items;
	}
}

void AMainPlayerState::SetRandomRole()
{
	int8 Index = FMath::RandRange(1, 4);
	PlayerRole = static_cast<ECharacterRole>(Index);
	UE_LOG(LogTemp, Warning, TEXT("[PLAYER STATE] %s has Random role %d"), *GetPersistantId(), Index);
}

FString AMainPlayerState::GetPersistantId()
{
#if WITH_EDITOR
	if (GIsEditor)
	{
		if (!GetWorld())
		{
			UE_LOG(LogTemp, Warning, TEXT("NO WORLD"));
			return TEXT("UNKNOWN");
		}
		if (AGameStateBase* GS = GetWorld()->GetGameState())
		{
			int32 Index = GS->PlayerArray.IndexOfByKey(this);
			return FString::Printf(TEXT("PLAYER%02d"), Index);
		}
	}
#endif
	
	if (!GetPlayerName().IsEmpty())
	{
		return GetPlayerName();
	}
	
	if (GetUniqueId().IsValid())
	{
		return GetUniqueId()->ToString();
	}

	return TEXT("UNKNOWN");
}

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
	
	DOREPLIFETIME(AMainPlayerState, IsReady);
	DOREPLIFETIME(AMainPlayerState, PlayerRole);
	DOREPLIFETIME(AMainPlayerState, Items);
}
