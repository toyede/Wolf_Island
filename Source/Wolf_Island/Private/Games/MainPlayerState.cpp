// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/MainPlayerState.h"

#include "Net/UnrealNetwork.h"

void AMainPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMainPlayerState, PlayerTag);
}
