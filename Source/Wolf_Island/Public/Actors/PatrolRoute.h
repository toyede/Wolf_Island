// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PatrolRoute.generated.h"

class USplineComponent;

UCLASS()
class WOLF_ISLAND_API APatrolRoute : public AActor
{
	GENERATED_BODY()
	
public:	
	APatrolRoute();

	TObjectPtr<USplineComponent> SplinePoints;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
