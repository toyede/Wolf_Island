// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AttackCollisionComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnHitActor, const FHitResult&);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WOLF_ISLAND_API UAttackCollisionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FOnHitActor OnHitActor;

	UPROPERTY(EditAnywhere, Category = "Trace")
	FName TraceStartSocketName;

	UPROPERTY(EditAnywhere, Category = "Trace")
	FName TraceEndSocketName;

	UPROPERTY(EditAnywhere, Category = "Trace")
	float TraceRadius = 20.f;

protected:
	UPROPERTY(EditAnywhere, Category = "Trace")
	TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;

	UPROPERTY(EditAnywhere, Category = "Trace")
	TArray<AActor*> IgnoredActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebug = false;

	UPROPERTY()
	TArray<AActor*> AlreadyHitActors;

	bool bIsCollisionEnabled = false;

	// 프레임 간 빈 공간 방지용
	FVector PrevTraceStart;
	FVector PrevTraceEnd;
	bool bHasPrevPosition = false;

public:
	UAttackCollisionComponent();

	void TurnOnCollision();
	void TurnOffCollision();
	void AddIgnoredActor(AActor* Actor);
	void RemoveIgnoredActor(AActor* Actor);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool CanHitActor(AActor* Actor) const;
	void CollisionTrace();
	void PerformTrace(const FVector& Start, const FVector& End, TArray<FHitResult>& OutHits);
	void ProcessHits(const TArray<FHitResult>& Hits);
};