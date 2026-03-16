// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/PreviewActor.h"

// Sets default values
APreviewActor::APreviewActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	RootComponent = MeshComponent;

	MeshComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	MeshComponent->SetGenerateOverlapEvents(true);

}

// Called when the game starts or when spawned
void APreviewActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APreviewActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APreviewActor::SetPreviewMesh(UStaticMesh* NewMesh)
{
	if (MeshComponent && NewMesh)
	{
		MeshComponent->SetStaticMesh(NewMesh);
        
		UMaterialInterface* BaseMat = MeshComponent->GetMaterial(0);
		if (BaseMat)
		{
			GhostMaterial = MeshComponent->CreateDynamicMaterialInstance(0, BaseMat);
		}
	}
}

void APreviewActor::UpdateGhostVisual_Implementation(bool bIsAvailable)
{
	if (GhostMaterial)
	{
		GhostMaterial->SetScalarParameterValue(TEXT("IsAvailable"), bIsAvailable ? 1.0f : 0.0f);
	}
}
