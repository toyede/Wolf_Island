// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/PlayerHUD.h"
#include "Widgets/Inventory/ItemAcquiredBlock.h"

#include "Character/MainPlayer.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"

void UPlayerHUD::AddItemMessage(FItemAddResult Result)
{
	if (Result.OperationResult == EItemAddedResult::NoItemAdded) return;
	
	if (ItemAcquiredBlockClass)
	{
		UItemAcquiredBlock* Block = CreateWidget<UItemAcquiredBlock>(this, ItemAcquiredBlockClass);

		if (Block)
		{
			const FString Info = {Result.ItemName.ToString()+" x"+FString::FromInt(Result.ActualAmountAdded)};

			Block->InfoText->SetText(FText::FromString(Info));
			UE_LOG(LogTemp, Warning, TEXT("%s"),*Info);
			if (InfoList)
			{
				InfoList->InsertChildAt(0, Block);
			}
		}
	}
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

		if (InteractionBar)
		{
			if (UKismetSystemLibrary::K2_IsTimerActiveHandle(this, InteractionTimer))
			{
				InteractionBar->SetPercent(ElapsedTime/TotalTime);
			} else
			{
				InteractionBar->SetPercent(0);
			}
		}
	}
}
