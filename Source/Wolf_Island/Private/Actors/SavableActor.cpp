// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/SavableActor.h"

#include "Net/UnrealNetwork.h"

// Sets default values
ASavableActor::ASavableActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	if (!GUID.IsValid())
	{
		GUID = FGuid::NewGuid();
	}

}

// Called when the game starts or when spawned
void ASavableActor::BeginPlay()
{
	Super::BeginPlay();
	
	
}

FGuid ASavableActor::GetGUID() const
{
	return GUID;
}

// Called every frame
void ASavableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASavableActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ASavableActor, GUID);
}

