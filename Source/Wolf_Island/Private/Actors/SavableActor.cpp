// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/SavableActor.h"

#include "Net/UnrealNetwork.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

// Sets default values
ASavableActor::ASavableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

}

// Called when the game starts or when spawned
void ASavableActor::BeginPlay()
{
	Super::BeginPlay();
	
	/*if (HasAuthority() && !GUID.IsValid())
	{
		GUID = FGuid::NewGuid();
	}*/

	
}

void ASavableActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (!GUID.IsValid())
	{
		GUID = FGuid::NewGuid();
	}
}

void ASavableActor::PostDuplicate(EDuplicateMode::Type DuplicateMode)
{
	Super::PostDuplicate(DuplicateMode);
	
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		GUID = FGuid::NewGuid();
		Modify();
	}
}

FGuid ASavableActor::GetGUID() const
{
	return GUID;
}

// Called every frame
void ASavableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	FString value = GUID.ToString();
	
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			DeltaTime,
			FColor::Green,
			FString::Printf(TEXT("[%s] GUID: %s"), *GetName(), *value)
		);
	}*/
}

void ASavableActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ASavableActor, GUID);
}

void ASavableActor::SaveData_Implementation(FActorSaveData& OutData)
{
	ISaveInterface::SaveData_Implementation(OutData);
	OutData.ActorID = GUID;
	OutData.ActorClass = GetClass();
	OutData.Transform = GetTransform();
	OutData.Velocity = GetVelocity();
	
	FMemoryWriter Writer(OutData.BinaryData, true);
	FObjectAndNameAsStringProxyArchive Ar(Writer, true);
	Ar.ArIsSaveGame = true;

	Serialize(Ar);
}

void ASavableActor::LoadData_Implementation(const FActorSaveData& InData)
{
	ISaveInterface::LoadData_Implementation(InData);
	GUID = InData.ActorID;
	SetActorTransform(InData.Transform);
	
	FMemoryReader Reader(InData.BinaryData, true);
	FObjectAndNameAsStringProxyArchive Ar(Reader, true);
	Ar.ArIsSaveGame = true;
	
	Serialize(Ar);
}

