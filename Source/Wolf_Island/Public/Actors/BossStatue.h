// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BossWall.h"
#include "NiagaraComponent.h"
#include "Interaction/InteractionInterface.h"
#include "BossStatue.generated.h"

class AEnemyAIBoss;
class UBoxComponent;
class UStatusComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStatueDestroyed);

UCLASS()
class WOLF_ISLAND_API ABossStatue : public ABossWall, public IInteractionInterface
{
	GENERATED_BODY()

public:
	ABossStatue();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// BP 호환성 유지용 (기존 BP 레퍼런스 보존 - 인터랙션 방식으로 전환 후 제거 예정)
	UPROPERTY(EditDefaultsOnly, Category = "Statue|Status")
	UStatusComponent* StatusComponent;

public:
	UPROPERTY(BlueprintAssignable)
	FOnStatueDestroyed OnStatueDestroyed;

	// IInteractionInterface 구현
	// BlueprintNativeEvent → _Implementation 접미사로 오버라이드
	virtual void BeginFocus_Implementation() override;
	virtual void EndFocus_Implementation() override;
	virtual void Interact_Implementation(AActor* Interactor) override;
	// 일반 virtual 함수
	virtual void BeginInteract() override;
	virtual void EndInteract() override;

	// 인터랙션(꾹 누르기) 지속 시간
	UPROPERTY(EditAnywhere, Category = "Statue|Interaction")
	float InteractionHoldDuration = 2.0f;

protected:
	UPROPERTY(EditAnywhere, Category = "Statue|Healing")
	float HealAmount = 50.f;

	UPROPERTY(EditAnywhere, Category = "Statue|Healing")
	float HealInterval = 2.0f;

	// 파괴 이펙트 (추후)
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