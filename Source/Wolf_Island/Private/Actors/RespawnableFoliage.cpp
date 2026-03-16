// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/RespawnableFoliage.h"

// Sets default values
ARespawnableFoliage::ARespawnableFoliage()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
	MeshComponent->SetCollisionProfileName(TEXT("BlockAll"));

}

void ARespawnableFoliage::InitializeFoliage(UInstancedStaticMeshComponent* InISMC, const FTransform& InTransform)
{
	OriginalISMC = InISMC;
	OriginalTransform = InTransform;

	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(
			RespawnTimerHandle, 
			this, 
			&ARespawnableFoliage::RespawnOriginalFoliage, 
			RespawnTime, 
			false
		);
	}
}

// Called when the game starts or when spawned
void ARespawnableFoliage::BeginPlay()
{
	Super::BeginPlay();
	
}

void ARespawnableFoliage::RespawnOriginalFoliage()
{
	if (HasAuthority() && OriginalISMC)
	{
		Multi_AddFoliageInstance(OriginalISMC, OriginalTransform);
		
		Destroy();
	}
}

void ARespawnableFoliage::Multi_AddFoliageInstance_Implementation(UInstancedStaticMeshComponent* TargetISMC,
	const FTransform& TargetTransform)
{
	if (TargetISMC)
	{
		TargetISMC->AddInstance(TargetTransform, true);
	}
}

// Called every frame
void ARespawnableFoliage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

