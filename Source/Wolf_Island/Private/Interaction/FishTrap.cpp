// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/FishTrap.h"

#include "Character/MainPlayerController.h"
#include "Components/InventoryComponent.h"
#include "Widgets/FishTrap/FishTrapScreen.h"
#include "Net/UnrealNetwork.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Kismet/GameplayStatics.h"

AFishTrap::AFishTrap()
{
	bReplicates = true;

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	InventoryComponent->SetSlotsCapacity(11);
}

void AFishTrap::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		ServerFishTimerEndTime = GetWorld()->GetTimeSeconds() + FishCatchInterval;
		GetWorldTimerManager().SetTimer(FishTimerHandle, this, &AFishTrap::TryCatchFish, FishCatchInterval, true);
	}
}

void AFishTrap::Interact(AActor* Interactor)
{
	if (HasAuthority())
	{
		if (APawn* InteractorPawn = Cast<APawn>(Interactor))
		{
			if (AMainPlayerController* PC = Cast<AMainPlayerController>(InteractorPawn->GetController()))
			{
				PC->Client_OpenFishTrapUI(this, Interactor);
			}
		}
	}
}

TArray<FName> AFishTrap::GetFishTrapRowNames() const
{
	TArray<FName> Result;
	if (FishTrapItemTable)
	{
		Result = FishTrapItemTable->GetRowNames();
	}
	return Result;
}

void AFishTrap::SaveData_Implementation(FActorSaveData& OutData)
{
	OutData.ActorID = GUID; 
	OutData.Transform = GetActorTransform();
	OutData.ActorClass = GetClass();
	
	FMemoryWriter Writer(OutData.BinaryData, true);
	FObjectAndNameAsStringProxyArchive Ar(Writer, true);
	Ar.ArIsSaveGame = true;

	InventoryComponent->Serialize(Ar);
}

void AFishTrap::LoadData_Implementation(const FActorSaveData& InData)
{
	GUID = InData.ActorID;
	SetActorTransform(InData.Transform);
	
	FMemoryReader Reader(InData.BinaryData, true);
	FObjectAndNameAsStringProxyArchive Ar(Reader, true);
	Ar.ArIsSaveGame = true;
	
	InventoryComponent->Serialize(Ar);
	InventoryComponent->SetInventoryContents(InventoryComponent->GetInventory());
	
	ForceNetUpdate();
}

void AFishTrap::Server_CloseFishTrap_Implementation()
{
}

void AFishTrap::TryCatchFish()
{
	if (!HasAuthority() || !InventoryComponent) return;

	ServerFishTimerEndTime = GetWorld()->GetTimeSeconds() + FishCatchInterval;

	if (bIsBaitActive)
	{
		BaitRemainingTime -= FishCatchInterval; 
		if (BaitRemainingTime <= 0.0f) bIsBaitActive = false;
	}

	if (!bIsBaitActive)
	{
		int32 BaitSlotIndex = 10;
		FItemSlot& BaitSlot = InventoryComponent->GetInventory()[BaitSlotIndex];
		if (BaitSlot.IsNotEmpty() && ValidBaitList.Contains(BaitSlot.ItemData.ItemID))
		{
			InventoryComponent->RemoveItemAmountAtSlot(BaitSlotIndex, 1);
			bIsBaitActive = true;
			BaitRemainingTime = BaitDuration; 
            
			ServerBaitTimerEndTime = GetWorld()->GetTimeSeconds() + BaitDuration;
			InventoryComponent->InventoryChanged();
		}
	}

	float BaseSuccessChance = 0.3f;
	float BaitBonusChance = 0.4f;
	float FinalChance = BaseSuccessChance + (bIsBaitActive ? BaitBonusChance : 0.0f);
    
	if (FMath::FRand() <= FinalChance && LootTable.Num() > 0)
	{
		FName RandomID = LootTable[FMath::RandRange(0, LootTable.Num() - 1)];
		FItemBaseData NewFish = InventoryComponent->CreateItemByID(RandomID, 1);

		if (NewFish.IsValid())
		{
			FItemAddResult Result = InventoryComponent->HandleAddItem(NewFish);
			if (Result.OperationResult != EItemAddedResult::NoItemAdded)
			{
				InventoryComponent->InventoryChanged();
			}
		}
	}
}

float AFishTrap::GetRemainingFishTime() const
{
	float CurrentServerTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	return FMath::Max(0.0f, ServerFishTimerEndTime - CurrentServerTime);
}

float AFishTrap::GetRemainingBaitTime() const
{
	if (!bIsBaitActive) return 0.0f;
    
	float CurrentServerTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	return FMath::Max(0.0f, ServerBaitTimerEndTime - CurrentServerTime);
}

void AFishTrap::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFishTrap, bIsBaitActive);
	DOREPLIFETIME(AFishTrap, BaitRemainingTime);
	DOREPLIFETIME(AFishTrap, ServerFishTimerEndTime);
	DOREPLIFETIME(AFishTrap, ServerBaitTimerEndTime);
}
