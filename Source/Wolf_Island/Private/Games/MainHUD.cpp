// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/MainHUD.h"
#include "Widgets/PlayerHUD.h"
#include "Widgets/Inventory/Inventory.h"

#include "Blueprint/UserWidget.h"

void AMainHUD::BeginPlay()
{
	Super::BeginPlay();

	if (PlayerHUDClass)
	{
		//PlayerHUDWidget = CreateWidget<UPlayerHUD>(GetWorld(), PlayerHUDClass);
		//PlayerHUDWidget->AddToViewport(-1);
	}

	if (InventoryWidgetClass)
	{
		InventoryWidget = CreateWidget<UInventory>(GetWorld(), InventoryWidgetClass);
		InventoryWidget->AddToViewport(1);
		InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void AMainHUD::DisplayHUD()
{
	IsHUDVisible = true;
	PlayerHUDWidget->SetVisibility(ESlateVisibility::Visible);
}

void AMainHUD::HideHUD()
{
	IsHUDVisible = false;
	PlayerHUDWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void AMainHUD::ToggleHUD()
{
	IsHUDVisible ? HideHUD() : DisplayHUD();
}

void AMainHUD::DisplayInventory()
{
	IsInventoryVisible = true;
	InventoryWidget->SetVisibility(ESlateVisibility::Visible);
}

void AMainHUD::HideInventory()
{
	IsInventoryVisible = false;
	InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void AMainHUD::ToggleInventory()
{
	IsInventoryVisible ? HideInventory() : DisplayInventory();
}