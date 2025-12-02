// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/WidgetComponent.h"
#include "AI/EnemyAIController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimMontage.h"
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
	class UStatusComponent* StatusComponent;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Form Change")
	TSubclassOf<UAnimInstance> HumanAnimBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Form Change")
	TSubclassOf<UAnimInstance> WolfAnimBP;

	UFUNCTION(BlueprintCallable)
	void ChangeForm(EEnemyForm Form);

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* ChangeFormMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackDamage;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USkeletalMeshComponent* FaceMesh;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USkeletalMeshComponent* TorsoMesh;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USkeletalMeshComponent* LegsMesh;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USkeletalMeshComponent* FeetMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* WolfMesh;

	UPROPERTY()
	TArray<USkeletalMeshComponent*> HumanParts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Form Change")
	TSubclassOf<UAnimInstance> HumanAnimBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Form Change")
	TSubclassOf<UAnimInstance> WolfAnimBP;

	UPROPERTY(EditAnywhere, Category = "Particle")
	class UNiagaraSystem* FormChangeNiagaraEffect;

	UFUNCTION()
	void SpawnParticle();

	UFUNCTION(BlueprintCallable)
	int32 GetNextPoint();

	UFUNCTION(BlueprintCallable)
	int32 GetRandomPointIndex();

	UPROPERTY(VisibleAnywhere)
	int32 CurrentPatrolIndex = 0;

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void StopAllMontages();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Form Change")
	bool bIsHuman = true;

	UFUNCTION(BlueprintCallable, Category = "Sound")
	void Growling();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* GrowlSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundAttenuation* AISoundAttenuation;
};
