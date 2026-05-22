// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/MainMenu/PauseMenu.h"

#include "Character/MainPlayerController.h"

void UPauseMenu::NativeConstruct()
{
	Super::NativeConstruct();
	
	SetIsFocusable(true);
	PlayerController = Cast<AMainPlayerController>(GetOwningPlayer());
}

FReply UPauseMenu::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	UE_LOG(LogTemp, Warning, TEXT("[PAUSE MENU] Key Down : %s"), *InKeyEvent.GetKey().ToString())
	if (InKeyEvent.GetKey()==EKeys::Escape)
	{
		if (PlayerController)
		{
			PlayerController->TogglePause();
			return FReply::Handled();
		}
	}
	
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPauseMenu::OnResumeClicked()
{
	//OnResumeButtonClicked.Broadcast();
}

void UPauseMenu::OnSettingClicked()
{
	//OnSettingButtonClicked.Broadcast();
}

void UPauseMenu::OnQuitClicked()
{
	//OnQuitButtonClicked.Broadcast();
}
