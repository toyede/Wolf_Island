// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "AI/AIControllers/EnemyAIController.h"
#include "AI/Interfaces/EnemyCommonInterface.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"

#include "Animation/AnimMontage.h"

#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"

#include "EnemyAIBase.generated.h"

class APatrolRoute;
class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitResponse);

UENUM(BlueprintType)
enum class EEnemyForm : uint8
{
	Human UMETA(DisplayName = "Human"),
	Wolf UMETA(DisplayName = "Wolf")
};

UCLASS()
class WOLF_ISLAND_API AEnemyAIBase : public ACharacter, public IEnemyCommonInterface
{
	GENERATED_BODY()

public:
	AEnemyAIBase();

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnHitResponse OnHitResponse;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol Route")
	TObjectPtr<APatrolRoute> AssignedPatrolRoute; // 이걸 바로 할당하진 않음

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Patrol Route")
	TObjectPtr<APatrolRoute> NativePatrolRoute; // 원주민일 때 쓸 패트롤루트 

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Patrol Route")
	TObjectPtr<APatrolRoute> WolfPatrolRoute; // 늑대일 때 쓸 패트롤루트 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	class UStatusComponent* StatusComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Stats")
	EEnemyForm EnemyForm;

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

	virtual void SetMovementSpeed_Implementation(EEnemyState State) override;

	virtual void ThrowObject_Implementation() override;

	virtual UAnimMontage* GetThrowMontage_Implementation() override;

	UFUNCTION()
	void HitResponse();

	UPROPERTY(BlueprintReadWrite, Category = "AI|Controller")
	TObjectPtr<AEnemyAIController> EnemyAIController;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Movement")
	float PassiveSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Form")
	float NativePatrolSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Form")
	float WolfPatrolSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AttackingSpeed = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DeadSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|ThrowObject")
	TSubclassOf<AActor> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|ThrowObject")
	TObjectPtr<UAnimMontage> ThrowMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|ThrowObject")
	TObjectPtr<USoundBase> ThrowSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|ThrowObject")
	FName ThrowSocketName = TEXT("hand_r");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Frozen")
	TObjectPtr<UAnimMontage> FrozenMontage_Native;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Frozen")
	TObjectPtr<UAnimMontage> FrozenMontage_Wolf;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Frozen")
	TObjectPtr<USoundBase> FrozenHitSound;

	UFUNCTION()
	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	UFUNCTION()
	void OnFrozenMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
