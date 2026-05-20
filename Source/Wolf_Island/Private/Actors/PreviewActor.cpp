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
	InitializeDynamicMaterials();
	UpdateBuildBounds();
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

		InitializeDynamicMaterials();
		UpdateBuildBounds();
	}
}

void APreviewActor::RefreshGhostMaterials()
{
	InitializeDynamicMaterials();
}

void APreviewActor::UpdateGhostVisual_Implementation(bool bIsAvailable)
{
	if (GhostMaterials.Num() == 0)
	{
		InitializeDynamicMaterials();
	}

	for (UMaterialInstanceDynamic* GhostMat : GhostMaterials)
	{
		if (GhostMat)
		{
			const FLinearColor GhostColor = bIsAvailable
				? FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)
				: FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
			GhostMat->SetVectorParameterValue(TEXT("GhostColor"), GhostColor);
		}
	}

	if (GhostMaterial)
	{
		const FLinearColor GhostColor = bIsAvailable
			? FLinearColor(1.0f, 1.0f, 1.0f, 1.0f)
			: FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
		GhostMaterial->SetVectorParameterValue(TEXT("GhostColor"), GhostColor);
	}
}

void APreviewActor::UpdateBuildProgress_Implementation(float Progress)
{
	if (GhostMaterials.Num() == 0)
	{
		InitializeDynamicMaterials();
	}

	UpdateBuildBounds();
	const float ClampedProgress = FMath::Clamp(Progress, 0.0f, 1.0f);

	for (UMaterialInstanceDynamic* GhostMat : GhostMaterials)
	{
		if (GhostMat)
		{
			GhostMat->SetScalarParameterValue(TEXT("BuildProgress"), ClampedProgress);
			GhostMat->SetScalarParameterValue(TEXT("BuildMinZ"), BuildMinZ);
			GhostMat->SetScalarParameterValue(TEXT("BuildMaxZ"), BuildMaxZ);
		}
	}
}

void APreviewActor::InitializeDynamicMaterials()
{
	GhostMaterials.Reset();
	GhostMaterial = nullptr;

	TArray<UStaticMeshComponent*> MeshComponents;
	GetComponents<UStaticMeshComponent>(MeshComponents);

	for (UStaticMeshComponent* MeshComp : MeshComponents)
	{
		if (!MeshComp)
		{
			continue;
		}

		const int32 MatCount = MeshComp->GetNumMaterials();
		for (int32 MatIndex = 0; MatIndex < MatCount; ++MatIndex)
		{
			if (UMaterialInterface* BaseMat = MeshComp->GetMaterial(MatIndex))
			{
				if (UMaterialInstanceDynamic* DynMat = MeshComp->CreateDynamicMaterialInstance(MatIndex, BaseMat))
				{
					GhostMaterials.Add(DynMat);
					if (!GhostMaterial)
					{
						GhostMaterial = DynMat;
					}
				}
			}
		}
	}
}

void APreviewActor::UpdateBuildBounds()
{
	const FBox Bounds = GetComponentsBoundingBox(true);
	if (Bounds.IsValid)
	{
		BuildMinZ = Bounds.Min.Z;
		BuildMaxZ = Bounds.Max.Z;
	}
}
