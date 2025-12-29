// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/StoneProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/DamageEvents.h"

AStoneProjectile::AStoneProjectile()
{
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;

    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);     // 물리 반응 없이 "조회"만
    Mesh->SetGenerateOverlapEvents(true);
    Mesh->SetNotifyRigidBodyCollision(false);
    Mesh->SetCollisionObjectType(ECC_WorldDynamic);

    ProjectileComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileComp->InitialSpeed = 2500.f;
    ProjectileComp->MaxSpeed = 2500.f;
    ProjectileComp->bRotationFollowsVelocity = true;
    ProjectileComp->ProjectileGravityScale = 0.4f; // 직선 원하면 0
    ProjectileComp->bAutoActivate = false;
    InitialLifeSpan = 1.f;
    ProjectileComp->SetUpdatedComponent(Mesh);

    bReplicates = true;
    SetReplicateMovement(true);

}

void AStoneProjectile::LaunchProjectile(const FVector& Direction, float Speed)
{
    if (ProjectileComp)
    {
        ProjectileComp->Velocity = Direction * Speed;
        ProjectileComp->Activate(true);
    }
}

void AStoneProjectile::BeginPlay()
{
    Super::BeginPlay();

    Mesh->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnProjectileOverlap);
}

void AStoneProjectile::OnProjectileOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!HasAuthority()) return;
    if (!OtherActor || OtherActor == GetOwner()) return;

    if (OtherActor->ActorHasTag("Player"))
    {
        /*if (GEngine)
        {
            const int32 bHasTag = (OtherActor ? (OtherActor->ActorHasTag(TEXT("Player")) ? 1 : 0) : -1);
            const int32 bAuth = (HasAuthority() ? 1 : 0);

            const FString Msg = FString::Printf(
                TEXT("Overlap %s Tag=%d HasAuth=%d"),
                *GetNameSafe(OtherActor),
                bHasTag,
                bAuth
            );

            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Emerald, Msg);
        }*/
        FDamageEvent DamageEvent;
        OtherActor->TakeDamage(Damage, DamageEvent, nullptr, this);
    }

    Destroy();
}

