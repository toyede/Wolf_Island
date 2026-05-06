#include "Actors/StoneProjectile.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/DamageEvents.h"
#include "Character/MainPlayer.h"

AStoneProjectile::AStoneProjectile()
{
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;

    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Mesh->SetCollisionObjectType(ECC_WorldDynamic);
    Mesh->SetCollisionResponseToAllChannels(ECR_Overlap);
    Mesh->SetGenerateOverlapEvents(true);

    ProjectileComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileComp->SetUpdatedComponent(Mesh);
    ProjectileComp->InitialSpeed = 2000.f;
    ProjectileComp->MaxSpeed = 3000.f;
    ProjectileComp->bRotationFollowsVelocity = true;
    ProjectileComp->ProjectileGravityScale = 0.4f;

    bReplicates = true;
    SetReplicateMovement(true);
    bAlwaysRelevant = false;
    bNetLoadOnClient = false;
    SetNetUpdateFrequency(30.0f);
    SetMinNetUpdateFrequency(15.0f);
}

void AStoneProjectile::BeginPlay()
{
    Super::BeginPlay();

    Mesh->OnComponentBeginOverlap.AddDynamic(this, &AStoneProjectile::OnOverlap);

    SetLifeSpan(LifeSpan);

    if (AActor* OwnerActor = GetOwner())
    {
        Mesh->IgnoreActorWhenMoving(OwnerActor, true);
    }
}

void AStoneProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
    Mesh->OnComponentBeginOverlap.RemoveDynamic(this, &AStoneProjectile::OnOverlap);
    
    Super::EndPlay(EndPlayReason);
}

void AStoneProjectile::Launch(const FVector& Direction, float Speed)
{
    if (!HasAuthority()) return;

    if (ProjectileComp)
    {
        ProjectileComp->Velocity = Direction.GetSafeNormal() * Speed;
    }
}

void AStoneProjectile::OnOverlap(
    UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!HasAuthority()) return;
    if (!OtherActor || OtherActor == GetOwner()) return;

    if (AMainPlayer* Player = Cast<AMainPlayer>(OtherActor))
    {
        FDamageEvent DamageEvent;
        OtherActor->TakeDamage(Damage, DamageEvent, nullptr, this);
    }

    Destroy();
}