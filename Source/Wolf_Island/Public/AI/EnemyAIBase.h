// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/WidgetComponent.h"
#include "AI/EnemyAIController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "EnemyAIBase.generated.h"

UENUM(BlueprintType)
enum class EEnemyForm : uint8
{
	Human UMETA(DisplayName = "Human"),
	Wolf UMETA(DisplayName = "Wolf")
};

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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Stats");
	EEnemyForm EnemyForm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol Route")
	AActor* PatrolRoute;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UWidgetComponent* HealthBarWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Form")
	USkeletalMesh* HumanMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Form")
	USkeletalMesh* WolfMesh;

	UFUNCTION(BlueprintCallable)
	void ChangeForm(EEnemyForm Form);
};
