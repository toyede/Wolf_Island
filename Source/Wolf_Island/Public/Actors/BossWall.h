// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BossWall.generated.h"

UCLASS()
class WOLF_ISLAND_API ABossWall : public AActor
{
	GENERATED_BODY()
	
public:	
	ABossWall();

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Mesh;
};
