// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/WidgetComponent.h"
#include "AI/EnemyAIController.h"
#include "EnemyAIBase.generated.h"

UCLASS()
class WOLF_ISLAND_API AEnemyAIBase : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyAIBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float CurrentHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Maxhealth;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UWidgetComponent* HealthBarWidget;
};
