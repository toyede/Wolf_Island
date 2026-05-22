// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AI/AIControllers/EnemyAIBossController.h"
#include "AI/Interfaces/AttackMeshProvider.h"
#include "AI/Interfaces/EnemyCommonInterface.h"
#include "EnemyAIBoss.generated.h"

class UAnimMontage;
class USoundBase;
class UAttackCollisionComponent;
class UStatusComponent;
class AStatueForewarning;
class ABossStatue;
class ASummonedWolf;
class AMainPlayer;
class UNiagaraSystem;
class UParticleSystem;

// --- Delegates ---
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

	// --- Components ---
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Boss|Components")
	TObjectPtr<UStatusComponent> StatusComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Boss|Components")
	TObjectPtr<UAttackCollisionComponent> AttackCollisionComponent;

	// --- Delegates ---
	UPROPERTY(BlueprintAssignable, Category = "Boss|Delegates")
	FOnBossAttackEnd OnBossAttackEnd;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Delegates")
	FOnBossRushEnd OnBossRushEnd;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Delegates")
	FOnBossGroggyEnd OnBossGroggyEnd;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Delegates")
	FOnSummonStatueEnd OnSummonStatueEnd;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Delegates")
	FOnSummonWolvesEnd OnSummonWolvesEnd;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Delegates")
	FOnThrustEnd OnThrustEnd;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Delegates")
	FOnSpecialAttackEnd OnSpecialAttackEnd;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Delegates")
	FOnSummonWolvesEnd OnSummonPrayerEnd;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Delegates")
	FOnPhaseChanged OnPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Delegates")
	FOnBossCombatStart OnBossCombatStart;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Delegates")
	FOnBossCombatEnd OnBossCombatEnd;

	// --- Basic Combat State ---
	UPROPERTY(BlueprintReadOnly, Category = "Boss|Combat")
	bool bIsCombatActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Combat", ReplicatedUsing = OnRep_IsDead)
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly, Category = "Boss|Combat")
	TArray<TObjectPtr<AMainPlayer>> BossParticipants;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Phase")
	int32 CurrentPhase = 1;

	// --- Execute Pattern Functions (AI Tasks need public access) ---
	UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
	void StartBossCombat();

	UFUNCTION(BlueprintCallable, Category = "Boss|Combat")
	void EndBossCombat();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|Attack")
	void ExecuteAttack(int32 AttackIndex);

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|Rush")
	void ExecuteRush();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|Groggy")
	void ExecuteGroggy();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|Groggy")
	void EndGroggy();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|Phase2")
	void ExecuteSummonStatue();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|Phase2")
	void ExecuteSummonPrayer();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|SummonWolves")
	void ExecuteSummonWolves();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|Special")
	void ExecuteThrust();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern|Special")
	void ExecuteSpecialAttack();

	/** AnimNotify_ThrustImpact 에서 호출 — 서버에서 충격파 판정/데미지/넉백 처리 */
	void OnThrustImpact();

	void SetCurrentDamage(float Damage) { CurrentDamage = Damage; }

	// --- Interface Implementations ---
	virtual USkeletalMeshComponent* GetAttackMesh() const override { return GetMesh(); }
	virtual UAttackCollisionComponent* GetAttackCollisionComponent() const override { return AttackCollisionComponent; }
	virtual void Die_Implementation() override;

protected:
	// --- AActor & ACharacter Overrides ---
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// --- [Pattern Settings: Basic Attack] ---
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Attack")
	TArray<UAnimMontage*> AttackMontages;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Attack")
	TArray<float> AttackDamages;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Attack")
	TArray<FName> AttackStartSockets;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Attack")
	TArray<FName> AttackEndSockets;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Attack")
	TArray<float> AttackRadiuses;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Attack")
	TArray<USoundBase*> AttackSounds;

	// --- [Pattern Settings: Rush] ---
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Rush")
	UAnimMontage* RushMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Rush")
	float RushDamage = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Rush")
	FName RushStartSocket;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Rush")
	FName RushEndSocket;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Rush")
	float RushRadius = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Rush")
	USoundBase* RushSound;

	// --- [Pattern Settings: Groggy] ---
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Groggy")
	UAnimMontage* GroggyMontage;

	UPROPERTY(EditAnywhere, Category = "Boss|Pattern|Groggy")
	float GroggyDuration = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Groggy")
	USoundBase* GroggySound;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Groggy")
	USoundBase* GroggyGetUpSound;

	// --- [Pattern Settings: Phase 2 / Statue] ---
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Phase2")
	UAnimMontage* SummonStatueMontage;

	UPROPERTY(EditAnywhere, Category = "Boss|Pattern|Phase2")
	TSubclassOf<AStatueForewarning> ForewarningClass;

	UPROPERTY(EditAnywhere, Category = "Boss|Pattern|Phase2")
	TSubclassOf<ABossStatue> StatueClass;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Phase2")
	TArray<TObjectPtr<AActor>> StatueSpawnPoints;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Phase2")
	AActor* StatueSpawnPoint;

	// Used by dynamic-spawned boss to discover level target points.
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Phase2")
	FName StatueSpawnTag = TEXT("StatueSpawn");

	UPROPERTY(EditAnywhere, Category = "Boss|Pattern|Phase2|Spawn")
	int32 MaxSpawnRetries = 3;

	UPROPERTY(EditAnywhere, Category = "Boss|Pattern|Phase2|Spawn")
	float SpawnRetryDelay = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Boss|Pattern|Phase2|Spawn")
	float SpawnSafetyRadius = 150.0f;

	/** 바닥 스냅 시 Z 오프셋 (cm). 피벗이 중앙이면 메시 반높이, 바닥 기준이면 0 */
	UPROPERTY(EditAnywhere, Category = "Boss|Pattern|Phase2|Spawn")
	float StatueGroundOffset = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Phase2")
	USoundBase* SummonStatueSound;

	// --- [Pattern Settings: Summon Wolves] ---
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|SummonWolves")
	UAnimMontage* SummonWolvesMontage;

	UPROPERTY(EditAnywhere, Category = "Boss|Pattern|SummonWolves")
	TSubclassOf<ASummonedWolf> SummonedWolfClass;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|SummonWolves")
	TArray<TObjectPtr<AActor>> SummonedWolfSpawnPoints;

	UPROPERTY(EditAnywhere, Category = "Boss|Pattern|SummonWolves")
	bool bUseFixedWolfSpawnPoints = false;

	UPROPERTY(EditAnywhere, Category = "Boss|Pattern|SummonWolves")
	int32 MinSummonWolfCount = 1;

	UPROPERTY(EditAnywhere, Category = "Boss|Pattern|SummonWolves")
	int32 MaxSummonWolfCount = 4;

	UPROPERTY(EditAnywhere, Category = "Boss|Pattern|SummonWolves")
	int32 BaseSummonWolfCount = 2;

	UPROPERTY(EditAnywhere, Category = "Boss|Pattern|SummonWolves")
	int32 MaxAliveSummonedWolves = 8;

	UPROPERTY(EditAnywhere, Category = "Boss|Pattern|SummonWolves")
	float WolfSpawnMinRadius = 250.f;

	UPROPERTY(EditAnywhere, Category = "Boss|Pattern|SummonWolves")
	float WolfSpawnMaxRadius = 900.f;

	UPROPERTY(EditAnywhere, Category = "Boss|Pattern|SummonWolves")
	float WolfSpawnBlockRadius = 80.f;

	UPROPERTY(EditAnywhere, Category = "Boss|Pattern|SummonWolves")
	int32 WolfSpawnSearchAttempts = 12;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|SummonWolves")
	USoundBase* SummonWolvesSound;

	// --- [Pattern Settings: Special Attacks] ---
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Special")
	UAnimMontage* ThrustMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Special")
	USoundBase* ThrustSound;

	/** 충격파 반경 (cm) */
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Thrust")
	float ThrustRange = 250.f;

	/** 수평 넉백 강도 */
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Thrust")
	float ThrustForce = 1200.f;

	/** 수직 넉백 강도 */
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Thrust")
	float UpwardForce = 300.f;

	/** 충격파 데미지 */
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Thrust")
	float ThrustImpactDamage = 20.f;

	/** 몽타주 재생 속도 (1.0 = 기본, 낮을수록 느림) */
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Thrust", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float ThrustMontagePlayRate = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Special")
	UAnimMontage* SpecialAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Special")
	UAnimMontage* HowlingMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Special")
	TArray<FName> SpecialAttackStartSockets;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Special")
	TArray<FName> SpecialAttackEndSockets;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Special")
	TArray<float> SpecialAttackRadiuses;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern|Special")
	USoundBase* SpecialAttackSound;

	// --- Common Combat Helpers ---
	UFUNCTION()
	void OnAttackHit(const FHitResult& HitResult);

	UFUNCTION()
	void OnRep_IsDead();

	void ApplyDeadState();

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Combat|Sound")
	USoundBase* DieSound;

	// --- Hit Effect (피격 이펙트) ---
	/** Niagara 이펙트 (우선 사용) */
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Combat|Effects")
	UNiagaraSystem* HitEffect;

	/** Cascade 이펙트 (HitEffect가 없을 때 사용) */
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Combat|Effects")
	UParticleSystem* HitEffectCascade;

	/** 이펙트를 붙일 소켓명 (비어있으면 피격 위치에 스폰) */
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Combat|Effects")
	FName HitEffectSocketName = NAME_None;

	/** 피격 사운드 (Unreliable Multicast로 재생) */
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Combat|Effects")
	USoundBase* HitSound;

	// --- Montage Callback Helpers ---
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnRushMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnGroggyMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnSummonStatueMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnSummonWolvesMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnThrustMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void OnSpecialAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

private:
	// --- Internal Pattern Logic (Phase 2) ---
	void SpawnStatueSequence();
	void OnForewarningComplete();
	void OnForewarningResolved(bool bAreaClear);
	void TrySpawnStatueWithRetry();
	bool IsSpawnAreaOccupied(const FVector& Location) const;
	AActor* SelectSpawnPoint();
	void RefreshStatueSpawnPointsFromTag();
	void ClearSpawnState();

	// --- Internal Pattern Logic (Wolves) ---
	void SpawnWolvesSequence();
	int32 ComputeDesiredWolfSpawnCount() const;
	void CleanupSummonedWolves();
	bool IsWolfSpawnBlocked(const FVector& Location) const;
	bool FindWolfSpawnLocation(FVector& OutLocation) const;

	// --- Internal State ---
	UPROPERTY()
	float CurrentDamage = 0.f;

	bool bPhase2Triggered = false;
	bool bPhase3Triggered = false;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Phase2HPThreshold = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Phase3HPThreshold = 0.3f;

	TWeakObjectPtr<AActor> PendingSpawnPoint;
	FTimerHandle SpawnRetryTimerHandle;
	int32 SpawnRetryCount = 0;
	int32 SpawnPointCursor = 0;

	FTimerHandle GroggyTimerHandle;

	UPROPERTY(Replicated)
	bool bIsRushing = false;

	TArray<TWeakObjectPtr<ASummonedWolf>> AliveSummonedWolves;

	// --- Networking: Unreliable Multicasts for Cosmetics ---
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayAttackMontage(int32 AttackIndex);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayRushMontage();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayGroggyMontage();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayGroggyGetUp();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlaySummonMontage();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlaySummonWolvesMontage();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_StopMontage(float BlendOut);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayThrustMontage();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlaySpecialAttackMontage();

	// --- Hit Effect ---
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayHitEffect(FVector HitLocation, FVector HitNormal);
};
