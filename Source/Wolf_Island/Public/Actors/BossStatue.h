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

	UStatusComponent* StatusComponent;
public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(BlueprintAssignable)
	FOnStatueDestroyed OnStatueDestroyed;

	// 러쉬 충돌 시 받는 추가 데미지 배율
	UPROPERTY(EditAnywhere, Category = "Statue|Combat")
	float RushDamageMultiplier = 3.0f;

protected:
	UPROPERTY(EditAnywhere, Category = "Statue|Healing")
	float HealAmount = 50.f;

	UPROPERTY(EditAnywhere, Category = "Statue|Healing")
	float HealInterval = 2.0f;

	// 파괴 이펙트 나중에
	UPROPERTY(EditAnywhere, Category = "Statue|Effects")
	class UNiagaraSystem* DestroyEffect;

	UPROPERTY(EditAnywhere, Category = "Collision")
	UBoxComponent* BoxCollision;

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