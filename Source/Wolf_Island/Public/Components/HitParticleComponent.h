// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HitParticleComponent.generated.h"


class UNiagaraSystem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WOLF_ISLAND_API UHitParticleComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHitParticleComponent();
	
	UPROPERTY(EditAnywhere, Category = "HitParticle")
	UNiagaraSystem* DefaultHitParticle;

	UPROPERTY(EditAnywhere, Category = "HitParticle")
	TMap<TEnumAsByte<EPhysicalSurface>, UNiagaraSystem*> SurfaceHitParticles;

	UPROPERTY(EditAnywhere, Category = "HitParticle")
	bool bAttachToHitBone = false;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION()
	void HandlePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy,
	FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName,
	FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser);
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multi_PlayHitParticle(FHitResult Hit);
	
	UFUNCTION(BlueprintCallable, Category = "HitParticle")
	void PlayHitParticleAt(const FHitResult& Hit);
};
