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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dead")
	TObjectPtr<USoundBase> DieSound;

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
