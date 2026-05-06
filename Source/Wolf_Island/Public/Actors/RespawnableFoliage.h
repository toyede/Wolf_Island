// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SavableActor.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "RespawnableFoliage.generated.h"

UCLASS()
class WOLF_ISLAND_API ARespawnableFoliage : public ASavableActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARespawnableFoliage();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage Settings")
	float RespawnTime = 60.0f;

	// 원본 폴리지 정보 저장
	void InitializeFoliage(UInstancedStaticMeshComponent* InISMC, const FTransform& InTransform);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void RespawnOriginalFoliage();

	UFUNCTION(NetMulticast, Reliable)
	void Multi_AddFoliageInstance(UInstancedStaticMeshComponent* TargetISMC, const FTransform& TargetTransform);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY()
	UInstancedStaticMeshComponent* OriginalISMC;

	FTransform OriginalTransform;

	FTimerHandle RespawnTimerHandle;

};
