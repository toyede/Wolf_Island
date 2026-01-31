// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AnimalBase.generated.h"

class UStatusComponent;

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

public:	
	virtual void Tick(float DeltaTime) override;

};
