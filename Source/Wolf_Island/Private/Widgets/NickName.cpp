// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/NickName.h"

#include "Components/TextBlock.h"
#include "Games/MainPlayerState.h"

void UNickName::UpdateName(AController* Controller)
{
	UE_LOG(LogTemp, Warning, TEXT("[NICKNAME WIDGET] UPDATE NICKNAME TRY"))
	if (Controller->PlayerState)
	{
		if (AMainPlayerState* PS = Cast<AMainPlayerState>(Controller->PlayerState))
		{
			FString Name = PS->GetPersistantId();
			NickName->SetText(FText::FromString(Name));
			UE_LOG(LogTemp, Warning, TEXT("[NICKNAME WIDGET] UPDATE NICKNAME -> %s"), *Name);
		}
	}
}

void UNickName::UpdateName(APlayerState* PlayerState)
{
	UE_LOG(LogTemp, Warning, TEXT("[NICKNAME WIDGET] UPDATE NICKNAME BY PS TRY"))
	if (AMainPlayerState* PS = Cast<AMainPlayerState>(PlayerState))
	{
		FString Name = PS->GetPersistantId();
		NickName->SetText(FText::FromString(Name));
		UE_LOG(LogTemp, Warning, TEXT("[NICKNAME WIDGET] UPDATE NICKNAME -> %s"), *Name);
	}
}
