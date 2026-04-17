// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AnimalBase.generated.h"

class UStatusComponent;
class APickup;

UCLASS()
class WOLF_ISLAND_API AAnimalBase : public ACharacter
{
	GENERATED_BODY()

public:
	AAnimalBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Comp")
	UStatusComponent* StatusComponent;

protected:
	virtual void BeginPlay() override;

	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void Die();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dead", ReplicatedUsing = OnRep_IsDead)
	bool bIsDead = false;

	UFUNCTION()
	void OnRep_IsDead();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void ApplyDeadState();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayHitSound();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayDieSound();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundBase> DieSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound", meta = (ClampMin = "0.0"))
	float DieSoundVolumeMultiplier = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound", meta = (ClampMin = "0.0"))
	float HitSoundVolumeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound", meta = (ClampMin = "0.0"))
	float HitSoundCooldown = 0.15f;

	float LastHitSoundTime = -10000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dead|DropItem")
	TSubclassOf<APickup> DropItemClass;

	UPROPERTY(EditDefaultsOnly, Category = "Dead|DropItem")
	FDataTableRowHandle DropItemHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Dead|DropItem")
	int32 MinDropAmount = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Dead|DropItem")
	int32 MaxDropAmount = 1;

	void DropItem();

public:	
	virtual void Tick(float DeltaTime) override;

};
