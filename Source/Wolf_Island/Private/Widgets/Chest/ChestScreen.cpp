// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Chest/ChestScreen.h"

#include "Interaction/Chest.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Chest/ChestPanel.h"

void UChestScreen::NativeConstruct()
{
	Super::NativeConstruct();

	FInputModeUIOnly UIOnly;
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PC->SetInputMode(UIOnly);
	PC->bShowMouseCursor = true;
	
	SetKeyboardFocus();
}


FReply UChestScreen::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape || InKeyEvent.GetKey() == EKeys::Tab)
	{
		FInputModeGameOnly GameOnly;
		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		PC->SetInputMode(GameOnly);
		PC->bShowMouseCursor = false;
		ChestRef->IsOccupied = false;
		RemoveFromParent();
	}
	
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UChestScreen::InitializeChest(AChest* Chest, AActor* Interactor)
{
	ChestRef = Chest;
	ChestPanel->SetInventoryComponent(Chest, Interactor);
	ChestPanel->RefreshChest();	
}