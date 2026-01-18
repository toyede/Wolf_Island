// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Chest.h"

#include "Components/InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/Chest/ChestScreen.h"

AChest::AChest()
{
	//SetReplicates(true);
	bReplicates = true;
	
	ChestMesh = CreateDefaultSubobject<UStaticMeshComponent>("ChestMesh");
	ChestCoverMesh = CreateDefaultSubobject<UStaticMeshComponent>("ChestCoverMesh");
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("InventoryComponent");

	InventoryComponent->SetSlotsCapacity(ChestSlotsSize);
	InventoryComponent->SetWeightCapacity(ChestWeightCapacity);
}

void AChest::OpenChest()
{
	
}

void AChest::CloseChest()
{
	
}

//상자를 누군가 열었다!
void AChest::Interact(AActor* Interactor)
{
	//누가 사용 중이면 아무것도 안함
	if (IsOccupied) return;

	//사용 중이라고 설정
	IsOccupied = true;
	
	if (ChestCoverMesh && ChestSound)
	{
		UGameplayStatics:: PlaySound2D(GetWorld(), ChestSound);
	}
		
	//상자 위젯 생성
	UChestScreen* ChestScreen = CreateWidget<UChestScreen>(GetWorld(), ChestWidgetClass);
	ChestScreen->InitializeChest(this, Interactor);
	ChestScreen->SetIsFocusable(true);
	ChestScreen->AddToViewport();
}

void AChest::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AChest, IsOccupied);
}
