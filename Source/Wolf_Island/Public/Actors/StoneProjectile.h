// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StoneProjectile.generated.h"

UCLASS()
class WOLF_ISLAND_API AStoneProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AStoneProjectile();

	UFUNCTION(BlueprintCallable)
	void LaunchProjectile(const FVector& Direction, float Speed);

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileComp;
};
