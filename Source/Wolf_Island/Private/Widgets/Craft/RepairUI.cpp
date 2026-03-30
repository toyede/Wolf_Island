// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Craft/RepairUI.h"
#include "Widgets/Craft/RepairPanel.h"
#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "Input/Reply.h"
#include "Interaction/Repair_Actor.h" 

void URepairUI::InitRepairWindow(ARepair_Actor* InActor)
{
if (WBP_RepairPanel)
    {
        WBP_RepairPanel->InitRepairPanel(InActor);
    }
}

void URepairUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (TargetActor && WBP_RepairPanel)
	{
		WBP_RepairPanel->InitRepairPanel(TargetActor);
		UE_LOG(LogTemp, Log, TEXT("RepairUI: NativeConstruct에서 자동으로 Actor 연결 완료!"));
	}

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetShowMouseCursor(true);
		PC->SetIgnoreLookInput(true);
		PC->SetIgnoreMoveInput(true);
		
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(this->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        
		PC->SetInputMode(InputMode);
	}
	
	SetIsFocusable(true);
	SetKeyboardFocus();
}

void URepairUI::NativeDestruct()
{
	Super::NativeDestruct();

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetShowMouseCursor(false);
		PC->SetIgnoreLookInput(false);
		PC->SetIgnoreMoveInput(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
}

FReply URepairUI::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Tab || InKeyEvent.GetKey() == EKeys::Escape)
	{
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}