// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Trap_Base.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class AEnemyAIBase;

UCLASS()
class WOLF_ISLAND_API ATrap_Base : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATrap_Base();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> TrapMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	TObjectPtr<USphereComponent> TriggerSphere;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trap")
	float TrapSpan;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION(BlueprintCallable)
	void ReleaseTrap();

private:
	FTimerHandle ReleaseTimerHandle;
	
	TObjectPtr<AEnemyAIBase> TrappedEnemy;

};
