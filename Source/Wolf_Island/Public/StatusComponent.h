// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatusComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WOLF_ISLAND_API UStatusComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStatusComponent();

	//체력
	UPROPERTY(EditAnywhere)
	float CurrentHP;
	UPROPERTY(EditAnywhere)
	float MaxHP;

	//스태미나
	UPROPERTY(EditAnywhere)
	float CurrentStamina;
	UPROPERTY(EditAnywhere)
	float MaxStamina;

	//배고픔
	UPROPERTY(EditAnywhere)
	float CurrentHunger;
	UPROPERTY(EditAnywhere)
	float MaxHunger;

	//수분
	UPROPERTY(EditAnywhere)
	float CurrentHydration;
	UPROPERTY(EditAnywhere)
	float MaxHydration;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
