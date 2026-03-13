// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/PlayerHUD.h"
#include "Widgets/Inventory/ItemAcquiredBlock.h"

#include "Character/MainPlayer.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/StatusComponent.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Widgets/Inventory/HotbarSlot.h"
#include "Widgets/Inventory/InventorySlot.h"

void UPlayerHUD::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	PlayerRef = Cast<AMainPlayer>(GetOwningPlayerPawn());
	DisplayDefault();
	HideAirBar();
	HideInteraction();
}

void UPlayerHUD::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshHotBar();
}

void UPlayerHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	//인터랙션 바 업데이트
	UpdateInteraction();
	
	UpdateStatusBars();
}

void UPlayerHUD::AddItemMessage(FItemAddResult Result)
{
	if (ItemAcquiredBlockClass)
	{
		UItemAcquiredBlock* Block = CreateWidget<UItemAcquiredBlock>(this, ItemAcquiredBlockClass);

		if (Block)
		{
			FString Info = {Result.ItemName.ToString()+" x"+FString::FromInt(Result.ActualAmountAdded)};

			if (Result.OperationResult == EItemAddedResult::NoItemAdded)
			{
				switch (Result.OperationFailReason)
				{
					case EItemFailReason::SlotOverflow:
						Info = {"Full of inventory"};
						break;
					case EItemFailReason::WeightOverflow:
						Info = {"No more weight capacity."};
						break;
					case EItemFailReason::SystemError:
						Info = {"System error."};
						break;
					case EItemFailReason::NoReason:
						Info = {"No reason."};
						break;
				}
			}

			Block->InfoText->SetText(FText::FromString(Info));
			
			UE_LOG(LogTemp, Warning, TEXT("%s"),*Info);
			
			if (InfoList)
			{
				InfoList->InsertChildAt(0, Block);
			}
		}
	}
}

void UPlayerHUD::DisplayInteraction()
{
	ShowInteraction = true;
	InteractionBar->SetVisibility(ESlateVisibility::Visible);
	CrossHair->SetVisibility(ESlateVisibility::Hidden);
}

void UPlayerHUD::HideInteraction()
{
	ShowInteraction = false;
	InteractionBar->SetVisibility(ESlateVisibility::Hidden);
	CrossHair->SetVisibility(ESlateVisibility::Visible);
}

void UPlayerHUD::ToggleInteraction()
{
	ShowInteraction ? HideInteraction() : DisplayInteraction();
}

void UPlayerHUD::UpdateInteraction()
{
	if (PlayerRef && ShowInteraction)
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

void UPlayerHUD::DisplayInteractable()
{
	CrossHair->SetBrushFromTexture(InteractableCrossHair);
}

void UPlayerHUD::DisplayDefault()
{
	CrossHair->SetBrushFromTexture(DefaultCrossHair);
}

void UPlayerHUD::DisplayAirBar()
{
	AirBar->SetVisibility(ESlateVisibility::Visible);
}

void UPlayerHUD::HideAirBar()
{
	AirBar->SetVisibility(ESlateVisibility::Hidden);
}

void UPlayerHUD::RefreshHotBar()
{
	HotBar->ClearChildren();
	
	for (int i=0; i<6; i++)
	{
		UHotbarSlot* HotSlot = CreateWidget<UHotbarSlot>(this, SlotClass);
		HotSlot->SetDragDrop(false);
		HotSlot->SetInventoryRef(PlayerRef->InventoryComponent);
		HotSlot->SetIndex(i);
		HotSlot->SetSlotNumber(i+1);

		HotSlot->SetUnSelectedSlot();
		
		if (PlayerRef->HotBarIndex == i)
		{
			HotSlot->SetSelectedSlot();
		}
		
		HotSlot->RefreshSlot();
		HotBar->AddChildToWrapBox(HotSlot);
	}
}

void UPlayerHUD::UpdateHotBar()
{
	UE_LOG(LogTemp, Warning, TEXT("UpdateHotBar EXECUTED"));
	if (HotBar&&HotBar->HasAnyChildren())
	{
		int32 Count = HotBar->GetChildrenCount();
		
		for (int i=0; i<Count; i++){
			UHotbarSlot* HotSlot = Cast<UHotbarSlot>(HotBar->GetChildAt(i));
			HotSlot->SetUnSelectedSlot();
		
			if (PlayerRef->HotBarIndex == i)
			{
				HotSlot->SetSelectedSlot();
			}
			
			HotSlot->RefreshSlot();
		}
	}
}

void UPlayerHUD::UpdateStatusBars()
{
	if (PlayerRef)
	{
		if (UStatusComponent* Status = PlayerRef->StatusComponent)
		{
			HealthBar->SetPercent(Status->GetHPPercent());
			StaminaBar->SetPercent(Status->GetStaminaPercent());
			HungerBar->SetPercent(Status->GetHungerPercent());
			HydrationBar->SetPercent(Status->GetHydrationPercent());
			AirBar->SetPercent(Status->GetAirPercent());
		}
	}
}
