// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AttackCollisionComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AI/Enemy_Character/EnemyAIBase.h"

UAttackCollisionComponent::UAttackCollisionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	TraceObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

}


void UAttackCollisionComponent::BeginPlay()
{
	Super::BeginPlay();

	
}


void UAttackCollisionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsCollisionEnabled)
	{
		CollisionTrace();
	}
}

void UAttackCollisionComponent::TurnOnCollision()
{
	AlreadyHitActors.Empty();
	bIsCollisionEnabled = true;
}

void UAttackCollisionComponent::TurnOffCollision()
{
	bIsCollisionEnabled = false;
}

void UAttackCollisionComponent::AddIgnoredActor(AActor* Actor)
{
	IgnoredActors.Add(Actor);
}

void UAttackCollisionComponent::RemoveIgnoredActor(AActor* Actor)
{
	IgnoredActors.Remove(Actor);
}

bool UAttackCollisionComponent::CanHitActor(AActor* Actor) const
{
	return AlreadyHitActors.Contains(Actor) == false;
}

void UAttackCollisionComponent::CollisionTrace()
{
    IAttackMeshProvider* MeshProvider = Cast<IAttackMeshProvider>(GetOwner());
    if (!MeshProvider) return;

    USkeletalMeshComponent* AttackMesh = MeshProvider->GetAttackMesh();
    if (!AttackMesh) return;

    const FVector Start = AttackMesh->GetSocketLocation(TraceStartSocketName);
    const FVector End = AttackMesh->GetSocketLocation(TraceEndSocketName);

    TArray<FHitResult> OutHits;
    bool const bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
        GetOwner(),
        Start,
        End,
        TraceRadius,
        TraceObjectTypes,
        false,
        IgnoredActors,
        DrawDebugType,
        OutHits,
        true);

    if (bHit)
    {
        for (const FHitResult& Hit : OutHits)
        {
            AActor* HitActor = Hit.GetActor();
            if (HitActor && CanHitActor(HitActor))
            {
                AlreadyHitActors.Add(HitActor);
                if (OnHitActor.IsBound())
                {
                    OnHitActor.Broadcast(Hit);
                }
            }
        }
    }
}

