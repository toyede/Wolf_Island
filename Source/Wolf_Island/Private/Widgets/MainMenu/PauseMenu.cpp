// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/MainMenu/PauseMenu.h"

#include "Character/MainPlayerController.h"

void UPauseMenu::NativeConstruct()
{
	Super::NativeConstruct();
	
	PlayerController = Cast<AMainPlayerController>(GetOwningPlayer());
	
}

FReply UPauseMenu::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
	
	if (InKeyEvent.GetKey()==FKey("ESC"))
	{
		
	}
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
