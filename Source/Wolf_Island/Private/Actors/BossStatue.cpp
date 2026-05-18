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
	// BossRush 채널 그대로 유지 (돌진 감지 목적) - 콜리전 설정은 BP/프로젝트 세팅 따름

	// 플레이어 물리 차단 전용 컴포넌트 (WorldDynamic → Pawn 기본 Block)
	BlockingCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BlockingCollision"));
	BlockingCollision->SetupAttachment(RootComponent);
	BlockingCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BlockingCollision->SetCollisionObjectType(ECC_WorldDynamic);
	BlockingCollision->SetCollisionResponseToAllChannels(ECR_Block);
	BlockingCollision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	BlockingCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

	bReplicates = true;
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
	GetWorldTimerManager().ClearAllTimersForObject(this);
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
		//GEngine->AddOnScreenDebugMessage(
		//	-1,
		//	2.f,
		//	FColor::Green,
		//	FString::Printf(TEXT("Boss Healed: %.2f   Current HP: %.2f"), HealAmount, CachedBoss->StatusComponent->CurrentHP)
		///);

		if (HealEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				HealEffect,
				CachedBoss->GetMesh(), // ������ �޽��� ����
				HealEffectSocketName,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::KeepRelativeOffset,
				true
			);
		}
	}
}

float ABossStatue::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (DamageCauser && DamageCauser->IsA(AEnemyAIBoss::StaticClass()))
	{
		// ���� ������ ��� �߰� ������ ����
		ActualDamage *= RushDamageMultiplier;
	}

	StatusComponent->DecreaseHP(ActualDamage);

	//GEngine->AddOnScreenDebugMessage(
	//	-1,
	//	2.f,
	//	FColor::Red,
	//	FString::Printf(TEXT("Statue HP: %.2f"), StatusComponent->CurrentHP)
	//);

	if (StatusComponent->CurrentHP <= 0.f)
	{
		Die();
	}

	return ActualDamage;
}

void ABossStatue::Die()
{
	GetWorldTimerManager().ClearTimer(HealTimerHandle);

	// �ı� ����Ʈ
	if (DestroyEffect)
	{
		/*UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			DestroyEffect,
			GetActorLocation()
		);*/
		// ���̾ư��� ������
	}

	OnStatueDestroyed.Broadcast();

	Destroy();
}