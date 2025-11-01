// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/PlayerHUD.h"

#include "Character/MainPlayer.h"
#include "Components/ProgressBar.h"
#include "Kismet/KismetSystemLibrary.h"

void UPlayerHUD::AddItemMessage(FItemAddResult Result)
{
}

void UPlayerHUD::NativeConstruct()
{
	Super::NativeConstruct();
	
}

void UPlayerHUD::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	PlayerRef = Cast<AMainPlayer>(GetOwningPlayerPawn());
}

void UPlayerHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (PlayerRef)
	{
		FTimerHandle InteractionTimer = PlayerRef->InteractionTimer;
		float RemainingTime = UKismetSystemLibrary::K2_GetTimerRemainingTimeHandle(this, InteractionTimer);
		float ElapsedTime = UKismetSystemLibrary::K2_GetTimerElapsedTimeHandle(this, InteractionTimer);
		float TotalTime = RemainingTime + ElapsedTime;

		if (UKismetSystemLibrary::K2_IsTimerActiveHandle(this, InteractionTimer))
		{
			InteractionBar->SetPercent(ElapsedTime/TotalTime);
		} else
		{
			InteractionBar->SetPercent(0);
		}
	}
}
