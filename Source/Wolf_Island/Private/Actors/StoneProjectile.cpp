// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/StoneProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

AStoneProjectile::AStoneProjectile()
{
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;

    ProjectileComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileComp->InitialSpeed = 1500.f;
    ProjectileComp->MaxSpeed = 1500.f;
    ProjectileComp->bRotationFollowsVelocity = true;
    ProjectileComp->ProjectileGravityScale = 1.0f; // 직선 원하면 0
    InitialLifeSpan = 5.f;

}

void AStoneProjectile::LaunchProjectile(const FVector& Direction, float Speed)
{
    if (ProjectileComp)
    {
        ProjectileComp->Velocity = Direction * Speed;
        ProjectileComp->Activate(true); // 꼭 필요
    }
}

