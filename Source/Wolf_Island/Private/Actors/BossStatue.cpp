// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/BossStatue.h"
#include "AI/Enemy_Character/EnemyAIBoss.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Character/MainPlayer.h"
#include "Components/BoxComponent.h"
#include "Components/StatusComponent.h"
#include "NiagaraFunctionLibrary.h"

ABossStatue::ABossStatue()
{
	PrimaryActorTick.bCanEverTick = false;

	// BP 호환성 유지용 (기존 BP 레퍼런스 보존)
	StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxCollision->SetupAttachment(RootComponent);
	// BossRush 채널 그대로 유지 (돌진 감지 목적)

	// 플레이어 물리 차단 전용 컴포넌트 (WorldDynamic → Pawn 기본 Block)
	BlockingCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BlockingCollision"));
	BlockingCollision->SetupAttachment(RootComponent);
	BlockingCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BlockingCollision->SetCollisionObjectType(ECC_WorldDynamic);
	BlockingCollision->SetCollisionResponseToAllChannels(ECR_Block);
	BlockingCollision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	// Visibility는 Block으로 유지 → 플레이어 인터랙션 라인트레이스에 감지됨

	bReplicates = true;

	// 인터랙션 데이터 초기화 (InteractionHoldDuration은 BeginPlay에서 적용)
	InteractableData.CanInteract = true;
}

void ABossStatue::BeginPlay()
{
	Super::BeginPlay();

	// 인터랙션 지속 시간 적용 (꾹 눌러야 파괴)
	InteractableData.InteractionDuration = InteractionHoldDuration;
	InteractableData.CanInteract = true;
	InteractableData.Name = FText::FromString(TEXT("Statue"));

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

// ============================================================
// IInteractionInterface 구현
// ============================================================

void ABossStatue::BeginFocus_Implementation()
{
	// 포커스 시 하이라이트 등 BP 확장 가능
}

void ABossStatue::EndFocus_Implementation()
{
}

void ABossStatue::BeginInteract()
{
	// 인터랙션 시작 시 연출 (필요 시 BP에서 확장)
}

void ABossStatue::EndInteract()
{
	// 인터랙션 취소 시 연출
}

void ABossStatue::Interact_Implementation(AActor* Interactor)
{
	if (!InteractableData.CanInteract) return;

	// 중복 호출 방지
	InteractableData.CanInteract = false;

	Die();
}

// ============================================================

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