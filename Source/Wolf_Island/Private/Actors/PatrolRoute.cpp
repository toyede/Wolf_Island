// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/PatrolRoute.h"
#include "Components/SplineComponent.h"

APatrolRoute::APatrolRoute()
{
	PrimaryActorTick.bCanEverTick = true;

	SplinePoints = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	RootComponent = SplinePoints;
}

void APatrolRoute::BeginPlay()
{
	Super::BeginPlay();
	
}

void APatrolRoute::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

