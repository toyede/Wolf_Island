// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Chest.h"

#include "Components/InventoryComponent.h"
#include "Widgets/Chest/ChestScreen.h"

AChest::AChest()
{
	ChestMesh = CreateDefaultSubobject<UStaticMeshComponent>("ChestMesh");
	ChestCoverMesh = CreateDefaultSubobject<UStaticMeshComponent>("ChestCoverMesh");
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("InventoryComponent");

	InventoryComponent->SetSlotsCapacity(ChestSlotsSize);
	InventoryComponent->SetWeightCapacity(ChestWeightCapacity);
}

//상자를 누군가 열었다!
void AChest::Interact(AActor* Interactor)
{
	if (IsOccupied) return;

	IsOccupied = true;

	UChestScreen* ChestScreen = CreateWidget<UChestScreen>(GetWorld(), ChestWidgetClass);
	ChestScreen->InitializeChest(this, Interactor);
	ChestScreen->AddToViewport();
	ChestScreen->SetIsFocusable(true);
}
