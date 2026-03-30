// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Torch.h"

#include "NiagaraComponent.h"
#include "Components/PointLightComponent.h"

// Sets default values
ATorch::ATorch()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	TorchLight = CreateDefaultSubobject<UPointLightComponent>("TorchLight");
	Stick = CreateDefaultSubobject<UStaticMeshComponent>("Stick");
	TorchFire = CreateDefaultSubobject<UNiagaraComponent>("TorchFire");
	
	SetRootComponent(Stick);
	TorchLight->SetupAttachment(Stick);
	TorchFire->SetupAttachment(Stick);
}

// Called when the game starts or when spawned
void ATorch::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATorch::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

