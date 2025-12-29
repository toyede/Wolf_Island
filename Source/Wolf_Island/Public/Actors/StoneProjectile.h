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

	UPROPERTY(EditAnywhere, Category = "Damage")
	float Damage;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMeshComponent* Mesh;

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	class UProjectileMovementComponent* ProjectileComp;

	UFUNCTION()
	void OnProjectileOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
