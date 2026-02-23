// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/BuildingInterface.h"
#include "PreviewActor.generated.h"

UCLASS()
class WOLF_ISLAND_API APreviewActor : public AActor, public IBuildingInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APreviewActor();

	virtual void UpdateGhostVisual_Implementation(bool bIsAvailable) override;

	void SetPreviewMesh(UStaticMesh* NewMesh);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY()
	UMaterialInstanceDynamic* GhostMaterial;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
