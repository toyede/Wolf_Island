// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/WolfBossStatue.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AWolfBossStatue::AWolfBossStatue()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StatueMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StatueMesh"));
	RootComponent = StatueMesh;

	MaxHealth = 100.f;
	CurrentHealth = MaxHealth;

}

// Called when the game starts or when spawned
void AWolfBossStatue::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWolfBossStatue::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWolfBossStatue::TakeDamage(float DamageAmount)
{
	CurrentHealth -= DamageAmount;

	if (CurrentHealth <= 0.f)
	{
		DestoryStatue();
	}
}

void AWolfBossStatue::DestoryStatue()
{
	Destroy();
}

