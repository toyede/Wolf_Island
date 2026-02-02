// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AI/AIControllers/EnemyAIBossController.h"
#include "AI/Interfaces/AttackMeshProvider.h"
#include "AI/Interfaces/EnemyCommonInterface.h"
#include "EnemyAIBoss.generated.h"

class UAnimMontage;
class UAttackCollisionComponent;
class UStatusComponent;
class AStatueForewarning;
class ABossStatue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossAttackEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossRushEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossGroggyEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSummonStatueEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnThrustEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpecialAttackEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, int32, NewPhase);

UCLASS()
class WOLF_ISLAND_API AEnemyAIBoss : public ACharacter, public IAttackMeshProvider, public IEnemyCommonInterface
{
	GENERATED_BODY()

public:
	AEnemyAIBoss();

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnBossAttackEnd OnBossAttackEnd;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnBossRushEnd OnBossRushEnd;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnBossGroggyEnd OnBossGroggyEnd;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnSummonStatueEnd OnSummonStatueEnd;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnThrustEnd OnThrustEnd;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnSpecialAttackEnd OnSpecialAttackEnd;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnPhaseChanged OnPhaseChanged;

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Comp")
	TObjectPtr<UStatusComponent> StatusComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Comp")
	TObjectPtr<UAttackCollisionComponent> AttackCollisionComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TArray<float> AttackDamages;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float RushDamage = 20.f;

	UPROPERTY()
	float CurrentDamage = 0.f;

	void SetCurrentDamage(float Damage) { CurrentDamage = Damage; }

	// 콜리전 소켓 및 반경

	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	TArray<FName> AttackStartSockets;

	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	TArray<FName> AttackEndSockets;

	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	FName RushStartSocket;

	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	FName RushEndSocket;

	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	TArray<FName> SpecialAttackStartSockets;

	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	TArray<FName> SpecialAttackEndSockets;

	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	TArray<float> AttackRadiuses;

	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	float RushRadius = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Collision")
	TArray<float> SpecialAttackRadiuses;

	// 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TArray<UAnimMontage*> AttackMontages;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* RushMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* GroggyMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* SummonStatueMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* ThrustMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* SpecialAttackMontage;

	// 실행 함수

	UFUNCTION()
	void ExecuteAttack(int32 AttackIndex);

	UFUNCTION()
	void ExecuteRush();

	UFUNCTION()
	void ExecuteGroggy();

	UFUNCTION()
	void EndGroggy();

	UFUNCTION()
	void ExecuteSummonStatue();

	UFUNCTION()
	void ExecuteThrust();

	UFUNCTION()
	void ExecuteSpecialAttack();

	virtual void Die_Implementation() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Phase")
	int32 CurrentPhase = 1;

	bool bPhase2Triggered = false;

	UPROPERTY(EditAnywhere, Category = "Boss|Phase2")
	TSubclassOf<AStatueForewarning> ForewarningClass;

	UPROPERTY(EditAnywhere, Category = "Boss|Phase2")
	TSubclassOf<ABossStatue> StatueClass;

	UPROPERTY(EditInstanceOnly, Category = "Boss|Phase2")
	AActor* StatueSpawnPoint;

	void SpawnStatueSequence();
	void OnForewarningComplete();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Dead", ReplicatedUsing = OnRep_IsDead)
	bool bIsDead = false;

protected:
	UFUNCTION()
	void OnRep_IsDead();

	void ApplyDeadState();

	USoundBase* DieSound;

protected:
	UFUNCTION()
	void OnAttackHit(const FHitResult& HitResult);

	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnRushMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnGroggyMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnSummonStatueMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnThrustMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnSpecialAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected:
	UPROPERTY(EditAnywhere, Category = "Groggy")
	float GroggyDuration = 1.f;

private:
	FTimerHandle GroggyTimerHandle;

	UPROPERTY(Replicated)
	bool bIsRushing = false;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttackMontage(int32 AttackIndex);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayRushMontage();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayGroggyMontage();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayGroggyGetUp();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySummonMontage();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopMontage(float BlendOut);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayThrustMontage();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySpecialAttackMontage();

public:
	virtual USkeletalMeshComponent* GetAttackMesh() const override
	{
		return GetMesh();
	}

	virtual UAttackCollisionComponent* GetAttackCollisionComponent() const override
	{
		return AttackCollisionComponent;
	}

protected:
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
};