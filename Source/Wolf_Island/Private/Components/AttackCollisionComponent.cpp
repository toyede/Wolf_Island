// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/AttackCollisionComponent.h"
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
	bHasPrevPosition = false;
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
	return !AlreadyHitActors.Contains(Actor);
}

void UAttackCollisionComponent::CollisionTrace()
{
	IAttackMeshProvider* MeshProvider = Cast<IAttackMeshProvider>(GetOwner());
	if (!MeshProvider) return;

	USkeletalMeshComponent* AttackMesh = MeshProvider->GetAttackMesh();
	if (!AttackMesh) return;

	const FVector CurrStart = AttackMesh->GetSocketLocation(TraceStartSocketName);
	const FVector CurrEnd = AttackMesh->GetSocketLocation(TraceEndSocketName);

	TArray<FHitResult> AllHits;

	// 현재 프레임: 무기 길이 방향
	PerformTrace(CurrStart, CurrEnd, AllHits);

	// 이전→현재 연결: 빈 공간 커버
	if (bHasPrevPosition)
	{
		PerformTrace(PrevTraceStart, CurrStart, AllHits);
		PerformTrace(PrevTraceEnd, CurrEnd, AllHits);
	}

	// 위치 저장
	PrevTraceStart = CurrStart;
	PrevTraceEnd = CurrEnd;
	bHasPrevPosition = true;

	ProcessHits(AllHits);
}

void UAttackCollisionComponent::PerformTrace(const FVector& Start, const FVector& End, TArray<FHitResult>& OutHits)
{
	TArray<FHitResult> Hits;

	const bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetOwner(),
		Start,
		End,
		TraceRadius,
		TraceObjectTypes,
		false,
		IgnoredActors,
		bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		Hits,
		true);

	if (bHit)
	{
		OutHits.Append(Hits);
	}
}

void UAttackCollisionComponent::ProcessHits(const TArray<FHitResult>& Hits)
{
	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && CanHitActor(HitActor))
		{
			AlreadyHitActors.Add(HitActor);
			OnHitActor.Broadcast(Hit);
		}
	}
}