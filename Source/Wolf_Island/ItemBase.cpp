// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemBase.h"

// Sets default values
AItemBase::AItemBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AItemBase::LoadItemDataFromTable(UDataTable* DataTable, FName RowName)
{
	if (!DataTable) return;

	FItemData* FoundData = DataTable->FindRow<FItemData>(RowName, TEXT("ItemBaseLookup"));

	if (FoundData)
	{
		ItemData = *FoundData;

		if (ItemData.Mesh)
		{
			UStaticMeshComponent* MeshCom = FindComponentByClass<UStaticMeshComponent>();
			if (MeshCom)
			{
				MeshCom->SetStaticMesh(ItemData.Mesh);
			}
		}

	}
}

// Called when the game starts or when spawned
void AItemBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

