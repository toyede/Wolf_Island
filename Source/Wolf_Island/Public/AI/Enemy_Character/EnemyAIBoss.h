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
class ASummonedWolf;
class AMainPlayer;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossAttackEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossRushEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossGroggyEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSummonStatueEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSummonWolvesEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnThrustEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpecialAttackEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPhaseChanged, int32, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossCombatStart, AEnemyAIBoss*, Boss);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossCombatEnd);

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
	FOnSummonWolvesEnd OnSummonWolvesEnd;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnThrustEnd OnThrustEnd;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnSpecialAttackEnd OnSpecialAttackEnd;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnSummonWolvesEnd OnSummonPrayerEnd;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnPhaseChanged OnPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnBossCombatStart OnBossCombatStart;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnBossCombatEnd OnBossCombatEnd;

public:
	UPROPERTY(BlueprintReadOnly)
	bool bIsCombatActive = false;

	UFUNCTION(BlueprintCallable)
	void StartBossCombat();

	UFUNCTION(BlueprintCallable)
	void EndBossCombat();

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TArray<TObjectPtr<AMainPlayer>> BossParticipants;

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* SummonWolvesMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* HowlingMontage;

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

	UFUNCTION()
	void ExecuteSummonWolves();

	UFUNCTION()
	void ExecuteSummonPrayer();

	virtual void Die_Implementation() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Phase")
	int32 CurrentPhase = 1;

	bool bPhase2Triggered = false;

	UPROPERTY(EditAnywhere, Category = "Boss|Phase2")
	TSubclassOf<AStatueForewarning> ForewarningClass;

		UPROPERTY(EditAnywhere, Category = "Boss|Phase2")
	TSubclassOf<ABossStatue> StatueClass;

	// Preferred fixed spawn points configured in editor.
	UPROPERTY(EditInstanceOnly, Category = "Boss|Phase2")
	TArray<TObjectPtr<AActor>> StatueSpawnPoints;

	// Backward compatibility fallback.
	UPROPERTY(EditInstanceOnly, Category = "Boss|Phase2")
	AActor* StatueSpawnPoint;

	UPROPERTY(EditAnywhere, Category = "Boss|Phase2|Spawn")
	int32 MaxSpawnRetries = 3;

	UPROPERTY(EditAnywhere, Category = "Boss|Phase2|Spawn")
	float SpawnRetryDelay = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Boss|Phase2|Spawn")
	float SpawnSafetyRadius = 150.0f;

	void SpawnStatueSequence();
	void OnForewarningComplete();
	void OnForewarningResolved(bool bAreaClear);
	void TrySpawnStatueWithRetry();
	bool IsSpawnAreaOccupied(const FVector& Location) const;
	AActor* SelectSpawnPoint();
	void ClearSpawnState();

	TWeakObjectPtr<AActor> PendingSpawnPoint;
	FTimerHandle SpawnRetryTimerHandle;
	int32 SpawnRetryCount = 0;
	int32 SpawnPointCursor = 0;

	UPROPERTY(EditAnywhere, Category = "Boss|SummonWolves")
	TSubclassOf<ASummonedWolf> SummonedWolfClass;

	UPROPERTY(EditInstanceOnly, Category = "Boss|SummonWolves")
	TArray<TObjectPtr<AActor>> SummonedWolfSpawnPoints;

	UPROPERTY(EditAnywhere, Category = "Boss|SummonWolves")
	bool bUseFixedWolfSpawnPoints = false;

	UPROPERTY(EditAnywhere, Category = "Boss|SummonWolves")
	int32 MinSummonWolfCount = 1;

	UPROPERTY(EditAnywhere, Category = "Boss|SummonWolves")
	int32 MaxSummonWolfCount = 4;

	UPROPERTY(EditAnywhere, Category = "Boss|SummonWolves")
	int32 BaseSummonWolfCount = 2;

	UPROPERTY(EditAnywhere, Category = "Boss|SummonWolves")
	int32 MaxAliveSummonedWolves = 8;

	UPROPERTY(EditAnywhere, Category = "Boss|SummonWolves")
	float WolfSpawnMinRadius = 250.f;

	UPROPERTY(EditAnywhere, Category = "Boss|SummonWolves")
	float WolfSpawnMaxRadius = 900.f;

	UPROPERTY(EditAnywhere, Category = "Boss|SummonWolves")
	float WolfSpawnBlockRadius = 80.f;

	UPROPERTY(EditAnywhere, Category = "Boss|SummonWolves")
	int32 WolfSpawnSearchAttempts = 12;

	void SpawnWolvesSequence();
	int32 ComputeDesiredWolfSpawnCount() const;
	void CleanupSummonedWolves();
	bool IsWolfSpawnBlocked(const FVector& Location) const;
	bool FindWolfSpawnLocation(FVector& OutLocation) const;

	TArray<TWeakObjectPtr<ASummonedWolf>> AliveSummonedWolves;

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
	void OnSummonWolvesMontageEnded(UAnimMontage* Montage, bool bInterrupted);
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
	void Multicast_PlaySummonWolvesMontage();

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


	virtual UAttackCollisionComponent* GetAttackCollisionComponent() const override
	{
		return AttackCollisionComponent;
	}

protected:
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
};
