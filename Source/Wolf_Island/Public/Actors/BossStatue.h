// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BossWall.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "BossStatue.generated.h"

class AEnemyAIBoss;
class UStatusComponent;
class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStatueDestroyed);

UCLASS()
class WOLF_ISLAND_API ABossStatue : public ABossWall
{
	GENERATED_BODY()

public:
	ABossStatue();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, Category = "Statue|Status")
	UStatusComponent* StatusComponent;
public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(BlueprintAssignable)
	FOnStatueDestroyed OnStatueDestroyed;

	// ���� �浹 �� �޴� �߰� ������ ����
	UPROPERTY(EditAnywhere, Category = "Statue|Combat")
	float RushDamageMultiplier = 3.0f;

protected:
	UPROPERTY(EditAnywhere, Category = "Statue|Healing")
	float HealAmount = 50.f;

	UPROPERTY(EditAnywhere, Category = "Statue|Healing")
	float HealInterval = 2.0f;

	// �ı� ����Ʈ ���߿�
	UPROPERTY(EditAnywhere, Category = "Statue|Effects")
	class UNiagaraSystem* DestroyEffect;

	// BossRush 채널 감지용 (돌진 히트 판정)
	UPROPERTY(EditAnywhere, Category = "Collision")
	UBoxComponent* BoxCollision;

	// 플레이어 물리 차단 전용 (WorldDynamic)
	UPROPERTY(EditAnywhere, Category = "Collision")
	UBoxComponent* BlockingCollision;

	UPROPERTY(EditAnywhere, Category = "Statue|Effects")
	class UNiagaraSystem* HealEffect;

	UPROPERTY(EditAnywhere, Category = "Statue|Effects")
	FName HealEffectSocketName = TEXT("Root");

private:
	void StartHealingTimer();
	void HealBoss();
	void Die();

	FTimerHandle HealTimerHandle;

	UPROPERTY()
	AEnemyAIBoss* CachedBoss;
};