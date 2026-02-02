// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/BossStatue.h"
#include "AI/Enemy_Character/EnemyAIBoss.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/StatusComponent.h"
#include "Character/MainPlayer.h"
#include "Components/BoxComponent.h"

ABossStatue::ABossStatue()
{
	PrimaryActorTick.bCanEverTick = false;

	StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxCollision->SetupAttachment(RootComponent);
}

void ABossStatue::BeginPlay()
{
	Super::BeginPlay();

	CachedBoss = Cast<AEnemyAIBoss>(UGameplayStatics::GetActorOfClass(GetWorld(), AEnemyAIBoss::StaticClass()));

	if (CachedBoss)
	{
		StartHealingTimer();
	}
}

void ABossStatue::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(HealTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void ABossStatue::StartHealingTimer()
{
	GetWorldTimerManager().SetTimer(
		HealTimerHandle,
		this,
		&ABossStatue::HealBoss,
		HealInterval,
		true
	);
}

void ABossStatue::HealBoss()
{
	if (CachedBoss)
	{
		CachedBoss->StatusComponent->IncreaseHP(HealAmount);
	}
}

float ABossStatue::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (DamageCauser && DamageCauser->IsA(AEnemyAIBoss::StaticClass()))
	{
		// 러쉬 공격인 경우 추가 데미지 적용
		ActualDamage *= RushDamageMultiplier;
	}

	StatusComponent->DecreaseHP(ActualDamage);

	GEngine->AddOnScreenDebugMessage(
		-1,
		2.f,
		FColor::Red,
		FString::Printf(TEXT("Statue HP: %.2f"), StatusComponent->CurrentHP)
	);

	if (StatusComponent->CurrentHP <= 0.f)
	{
		Die();
	}

	return ActualDamage;
}

void ABossStatue::Die()
{
	GetWorldTimerManager().ClearTimer(HealTimerHandle);

	// 파괴 이펙트
	if (DestroyEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			DestroyEffect,
			GetActorLocation()
		);
	}

	OnStatueDestroyed.Broadcast();

	Destroy();
}