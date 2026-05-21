// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/PlayerHUD.h"

#include "WaterBodyComponent.h"
#include "Widgets/Inventory/ItemAcquiredBlock.h"

#include "Character/MainPlayer.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/StatusComponent.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "Games/MainPlayerState.h"
#include "Item/Pickup.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Widgets/Inventory/HotbarSlot.h"
#include "Widgets/Inventory/InventorySlot.h"

void UPlayerHUD::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UPlayerHUD::NativeConstruct()
{
	Super::NativeConstruct();
	
	UE_LOG(LogTemp, Warning, TEXT("[PlayerHUD] NativeConstruct HUD CLASS = %s"), *GetClass()->GetName());
	
	PlayerRef = Cast<AMainPlayer>(GetOwningPlayerPawn());
	DisplayDefault();
	HideAirBar();
	HideInteraction();
	
	RefreshHotBar();
}

void UPlayerHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (!CrossHair)
	{
		UE_LOG(LogTemp, Warning, TEXT("CrossHair is GONE!!!"));
	}
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

void UPlayerHUD::SetPlayerRef(AMainPlayer* OwnerPlayer)
{
	PlayerRef = OwnerPlayer;
	
	if (PlayerRef)
	{
		if (PlayerRef->StatusComponent)
		{
			UE_LOG(LogTemp, Warning, TEXT("[PLAYER HUD] INFECTION BINDING COMPLETED"))
			//PlayerRef->StatusComponent->OnInfectionChanged.AddDynamic(this, &UPlayerHUD::OnInfectionChanged);
		}
	}
}

void UPlayerHUD::DisplayInteraction()
{
	//UE_LOG(LogTemp, Warning, TEXT("[PlayerHUD] DisplayInteraction | %s"), *GetName())
	
	ShowInteraction = true;
	
	if (InteractionBar)
	{
		InteractionBar->SetVisibility(ESlateVisibility::Visible);
	} else
	{
		//UE_LOG(LogTemp, Warning, TEXT("[PlayerHUD] NO InteractionBar | %s"), *GetName())
	}
	
	if (CrossHair)
	{
		CrossHair->SetVisibility(ESlateVisibility::Hidden);
	} else
	{
		//UE_LOG(LogTemp, Warning, TEXT("[PlayerHUD] NO CrossHair | %s"), *GetName())
	}
}

void UPlayerHUD::HideInteraction()
{
	//UE_LOG(LogTemp, Warning, TEXT("[PlayerHUD] HideInteraction | %s"), *GetName())
	
	ShowInteraction = false;
	
	if (InteractionBar)
	{
		InteractionBar->SetVisibility(ESlateVisibility::Hidden);
	} else
	{
		//UE_LOG(LogTemp, Warning, TEXT("[PlayerHUD] NO InteractionBar | %s"), *GetName())
	}
	
	if (CrossHair)
	{
		CrossHair->SetVisibility(ESlateVisibility::Visible);
	} else
	{
		//UE_LOG(LogTemp, Warning, TEXT("[PlayerHUD] NO CrossHair | %s"), *GetName())
	}
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
	//UE_LOG(LogTemp, Warning, TEXT("[PlayerHUD] DisplayInteractable | %s"), *GetName())
	
	if (CrossHair)
	{
		CrossHair->SetBrushFromTexture(InteractableCrossHair);
	} else
	{
		//UE_LOG(LogTemp, Warning, TEXT("[PlayerHUD] NO CROSSHAIR | %s"), *GetName())
	}
	
	if (InteractableIcon)
	{
		InteractableIcon->SetVisibility(ESlateVisibility::Visible);
	}
}

void UPlayerHUD::DisplayDefault()
{
	//UE_LOG(LogTemp, Warning, TEXT("[PlayerHUD] DisplayDefault | %s"), *GetName())
	
	if (CrossHair)
	{
		CrossHair->SetBrushFromTexture(DefaultCrossHair);
	} else
	{
		//UE_LOG(LogTemp, Warning, TEXT("[PlayerHUD] NO CROSSHAIR | %s"), *GetName())
	}
	
	if (InteractableIcon)
	{
		InteractableIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
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
	//UE_LOG(LogTemp, Warning, TEXT("[PlayerHUD] UpdateHotBar EXECUTED | %s"), *GetName())
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
	} else
	{
		//UE_LOG(LogTemp, Warning, TEXT("[PlayerHUD] No HotBar | %s"), *GetName())
	}
}

void UPlayerHUD::UpdateStatusBars()
{
	if (PlayerRef)
	{
		if (UStatusComponent* Status = PlayerRef->StatusComponent)
		{
			float Health = Status->GetHPPercent();
			float Stamina = Status->GetStaminaPercent();
			float Hunger = Status->GetHungerPercent();
			float Hydration = Status->GetHydrationPercent();
			float Air = Status->GetAirPercent();
			
			HealthBar->SetPercent(Health);
			StaminaBar->SetPercent(Stamina);
			HungerBar->SetPercent(Hunger);
			HydrationBar->SetPercent(Hydration);
			AirBar->SetPercent(Air);
			
			Health <= 0.1f ? PlayIconAnim(HealthIconAnimation) : StopIconAnim(HealthIconAnimation);
			Stamina <= 0.1f ? PlayIconAnim(StaminaIconAnimation) : StopIconAnim(StaminaIconAnimation);
			Hunger <= 0.1f ? PlayIconAnim(HungerIconAnimation) : StopIconAnim(HungerIconAnimation);
			Hydration <= 0.1f ?	PlayIconAnim(HydrationIconAnimation) : StopIconAnim(HydrationIconAnimation);
		}
	}
}

void UPlayerHUD::OnInfectionChanged()
{
	if (PlayerRef->StatusComponent)
	{
		if (PlayerRef->StatusComponent->IsInfected)
		{	
			UE_LOG(LogTemp, Warning, TEXT("[PLAYER HUD] Infection changed : true"));
			if (Infected)
			{
				PlayAnimation(Infected, 0, 0);
			}
		} else
		{
			UE_LOG(LogTemp, Warning, TEXT("[PLAYER HUD] Infection changed : false"));
			if (Infected)
			{
				PlayAnimation(Infected, 0, 1);
			}	
		}
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PLAYER HUD] NO STATUS ON PLAYER"));
	}
}

void UPlayerHUD::DisplayTargetHP(AActor* Target)
{
	//UE_LOG(LogTemp, Warning, TEXT("[PLAYER HUD] Display target HP"));
	if (UStatusComponent* Status = Target->GetComponentByClass<UStatusComponent>())
	{
		float HP = Status->GetHPPercent() * 100.0f;
		FText HPText = FText::Format(FText::FromString("{0}%"), FText::AsNumber(FMath::RoundToInt(HP)));
		TargetHPText->SetText(HPText);
		TargetHPText->SetVisibility(ESlateVisibility::Visible);
	} else
	{
		//UE_LOG(LogTemp, Warning, TEXT("[PLAYER HUD] Target has no Status"));
		TargetHPText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPlayerHUD::HideTargetHP()
{
	//UE_LOG(LogTemp, Warning, TEXT("[PLAYER HUD] Hide target HP"));
	TargetHPText->SetVisibility(ESlateVisibility::Collapsed);
}

void UPlayerHUD::DisplayInteractionInfoText(AActor* Target)
{
	if (AMainPlayer* TargetPlayer = Cast<AMainPlayer>(Target))
	{
		if (AMainPlayerState* PS = Cast<AMainPlayerState>(TargetPlayer->GetPlayerState()))
		{
			FString PlayerName = PS->GetPlayerName();
			FText Info = FText::Format(FText::FromString(TEXT("{0}이(가) 소생 중")), FText::FromString(PlayerName));
			InteractingInfoText->SetText(Info);
			InteractingInfoText->SetVisibility(ESlateVisibility::Visible);
		} else
		{
			UE_LOG(LogTemp, Warning, TEXT("[PLAYER HUD] NO PS"));
		}
	}
}

void UPlayerHUD::HideInteractionInfoText()
{
	InteractingInfoText->SetVisibility(ESlateVisibility::Collapsed);
}

void UPlayerHUD::DisplayInteractableInfoText(AActor* Target)
{
	if (!Target) return;
	
	if (AMainPlayer* Player = Cast<AMainPlayer>(Target))
	{
		InteractableInfoText->SetText(FText::FromString("살려주기"));
	} else if (APickup* Item = Cast<APickup>(Target))
	{
		const FItemBaseData ItemData= Item->GetItemData();
		InteractableInfoText->SetText(ItemData.ItemName);
	} else
	{
		InteractableInfoText->SetText(FText::FromString(""));
		HideInteractableInfoText();
	}
	
	InteractableInfoText->SetVisibility(ESlateVisibility::Visible);
}

void UPlayerHUD::DisplayInteractableInfoTextByItem(const FItemData& ItemData)
{
	InteractableInfoText->SetText(ItemData.TextData.Name);
	InteractableInfoText->SetVisibility(ESlateVisibility::Visible);
}

void UPlayerHUD::DisplayInteractableInfoTextByComponent(UActorComponent* Component)
{
	//일단 이건 물밖에 없어서 물로 고정
	if (Component)
	{
		InteractableInfoText->SetText(FText::FromString(TEXT("물 마시기")));
		InteractableInfoText->SetVisibility(ESlateVisibility::Visible);
	}
}

void UPlayerHUD::HideInteractableInfoText()
{
	InteractableInfoText->SetVisibility(ESlateVisibility::Collapsed);
}

void UPlayerHUD::PlayIconAnim(UWidgetAnimation* Anim)
{
	if (IsAnimationPlaying(Anim)) return;
	PlayAnimation(Anim, 0, 0, EUMGSequencePlayMode::Forward, 1, true);
}

void UPlayerHUD::StopIconAnim(UWidgetAnimation* Anim)
{
	if (IsAnimationPlaying(Anim)) StopAnimation(Anim);
}
