// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/MainPlayerState.h"

#include "GameFramework/GameStateBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

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
	
	DOREPLIFETIME(AMainPlayerState, PlayerRole);
	DOREPLIFETIME(AMainPlayerState, Items);
}
