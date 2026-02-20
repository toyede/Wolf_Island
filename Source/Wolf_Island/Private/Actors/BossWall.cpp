// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/BossWall.h"

// Sets default values
ABossWall::ABossWall()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WallMesh"));
	RootComponent = Mesh;
	Mesh->SetCollisionObjectType(ECC_GameTraceChannel1);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}
