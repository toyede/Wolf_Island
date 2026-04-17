// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/AIControllers/EnemyAIController.h"
#include "AI/Interfaces/AttackMeshProvider.h"
#include "AI/Interfaces/EnemyCommonInterface.h"
#include "GameFramework/Character.h"
#include "SummonedWolf.generated.h"

class UAttackCollisionComponent;
class UStatusComponent;
class UAnimMontage;
class USoundBase;
class AMainPlayer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSummonedWolfAttackEnd);

UCLASS()
class WOLF_ISLAND_API ASummonedWolf : public ACharacter, public IEnemyCommonInterface, public IAttackMeshProvider
{
	GENERATED_BODY()

public:
	ASummonedWolf();

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnSummonedWolfAttackEnd OnAttackEnd;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Comp")
	TObjectPtr<UStatusComponent> StatusComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Comp")
	TObjectPtr<UAttackCollisionComponent> AttackCollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackRange = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackCooldown = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FName AttackStartSocket = TEXT("AttackStart");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FName AttackEndSocket = TEXT("AttackEnd");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackTraceRadius = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Dead")
	TObjectPtr<USoundBase> DieSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float PassiveSpeed = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float CombatSpeed = 420.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DeadSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float ChaseUpdateInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float MoveAcceptanceRadius = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_IsDead, Category = "Combat|Dead")
	bool bIsDead = false;

	virtual void SetMovementSpeed_Implementation(EEnemyState State) override;
	virtual void ThrowObject_Implementation() override;
	virtual void Die_Implementation() override;
	virtual void NormalAttack_Implementation() override;
	virtual void Howling_Implementation() override;

	virtual USkeletalMeshComponent* GetAttackMesh() const override
	{
		return GetMesh();
	}

	virtual UAttackCollisionComponent* GetAttackCollisionComponent() const override
	{
		return AttackCollisionComponent;
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	void StartChaseLoop();
	void StopChaseLoop();
	void UpdateChaseAndCombat();
	AMainPlayer* PickRandomAlivePlayer() const;
	bool IsValidCombatTarget(const AActor* Target) const;
	void TryAttackTarget();
	void HandleAttackCooldownFinished();

	UFUNCTION()
	void OnAttackHit(const FHitResult& HitResult);

	UFUNCTION()
	void HandleHPZero();

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnRep_IsDead();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttackMontage();

	void ApplyDeadState();

	UPROPERTY()
	TWeakObjectPtr<AMainPlayer> CurrentTarget;

	FTimerHandle ChaseTimerHandle;
	FTimerHandle AttackCooldownTimerHandle;
	bool bCanAttack = true;
	bool bIsAttacking = false;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;
};
