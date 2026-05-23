// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Enemy_Character/SummonedWolf.h"
#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Character/MainPlayer.h"
#include "Components/AttackCollisionComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StatusComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

ASummonedWolf::ASummonedWolf()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -96.f));

	StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));
	AttackCollisionComponent = CreateDefaultSubobject<UAttackCollisionComponent>(TEXT("AttackCollisionComponent"));

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AAIController::StaticClass();

	bReplicates = true;
	SetReplicateMovement(true);
}

void ASummonedWolf::BeginPlay()
{
	Super::BeginPlay();

	if (AttackCollisionComponent)
	{
		AttackCollisionComponent->OnHitActor.AddUObject(this, &ASummonedWolf::OnAttackHit);
		AttackCollisionComponent->AddIgnoredActor(this);

		AActor* OwnerBoss = GetOwner();
		if (OwnerBoss)
		{
			AttackCollisionComponent->AddIgnoredActor(OwnerBoss);
		}
		else
		{
		// Owner가 아직 없을 경우 한 프레임 뒤에 재시도
		// [Refactor] 타이머 콜백: WeakLambda로 안전한 this 캡처
		GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (AttackCollisionComponent && GetOwner())
			{
				AttackCollisionComponent->AddIgnoredActor(GetOwner());
			}
		}));
		}
	}

	if (StatusComponent)
	{
		StatusComponent->OnHPZero.AddDynamic(this, &ASummonedWolf::HandleHPZero);
	}

	if (HasAuthority())
	{
		StartChaseLoop();
	}
}

void ASummonedWolf::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// [Refactor] 델리게이트 해제: AttackCollisionComponent OnHitActor 바인딩 대칭 해제
	if (AttackCollisionComponent)
	{
		AttackCollisionComponent->OnHitActor.RemoveAll(this);
	}

	// [Refactor] 타이머 정리 (GetWorld() null 체크: EndPlay 시점에 월드가 무효화될 수 있음)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}
	
	if (StatusComponent)
	{
		StatusComponent->OnHPZero.RemoveDynamic(this, &ASummonedWolf::HandleHPZero);
	}
	
	Super::EndPlay(EndPlayReason);
}

void ASummonedWolf::StartChaseLoop()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!GetWorldTimerManager().IsTimerActive(ChaseTimerHandle))
	{
		GetWorldTimerManager().SetTimer(
			ChaseTimerHandle,
			this,
			&ASummonedWolf::UpdateChaseAndCombat,
			ChaseUpdateInterval,
			true,
			0.1f
		);
	}
}

void ASummonedWolf::StopChaseLoop()
{
	GetWorldTimerManager().ClearTimer(ChaseTimerHandle);
}

AMainPlayer* ASummonedWolf::PickRandomAlivePlayer() const
{
	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMainPlayer::StaticClass(), Players);

	TArray<AMainPlayer*> AlivePlayers;
	for (AActor* Actor : Players)
	{
		AMainPlayer* Player = Cast<AMainPlayer>(Actor);
		if (!Player || !Player->StatusComponent)
		{
			continue;
		}

		if (Player->StatusComponent->CurrentHP > 0.f)
		{
			AlivePlayers.Add(Player);
		}
	}

	if (AlivePlayers.Num() == 0)
	{
		return nullptr;
	}

	const int32 Index = FMath::RandRange(0, AlivePlayers.Num() - 1);
	return AlivePlayers[Index];
}

bool ASummonedWolf::IsValidCombatTarget(const AActor* Target) const
{
	const AMainPlayer* Player = Cast<AMainPlayer>(Target);
	return Player && Player->StatusComponent && Player->StatusComponent->CurrentHP > 0.f;
}

void ASummonedWolf::UpdateChaseAndCombat()
{
	// [Refactor] 타이머 콜백: World 및 Dead 상태 유효성 체크
	if (!HasAuthority() || bIsDead || !GetWorld())
	{
		return;
	}

	if (!IsValidCombatTarget(CurrentTarget.Get()))
	{
		CurrentTarget = PickRandomAlivePlayer();
	}

	AAIController* AIC = Cast<AAIController>(GetController());
	if (!AIC || !CurrentTarget.IsValid())
	{
		return;
	}

	if (!bIsAttacking)
	{
		AIC->MoveToActor(CurrentTarget.Get(), MoveAcceptanceRadius);
	}

	TryAttackTarget();
}

void ASummonedWolf::TryAttackTarget()
{
	if (!CurrentTarget.IsValid() || !bCanAttack || bIsAttacking)
	{
		return;
	}

	const float DistSq = FVector::DistSquared(GetActorLocation(), CurrentTarget->GetActorLocation());
	if (DistSq > FMath::Square(AttackRange))
	{
		return;
	}

	bCanAttack = false;
	bIsAttacking = true;
	IEnemyCommonInterface::Execute_NormalAttack(this);

	GetWorldTimerManager().SetTimer(
		AttackCooldownTimerHandle,
		this,
		&ASummonedWolf::HandleAttackCooldownFinished,
		AttackCooldown,
		false
	);
}

void ASummonedWolf::HandleAttackCooldownFinished()
{
	bCanAttack = true;
}

void ASummonedWolf::SetMovementSpeed_Implementation(EEnemyState State)
{
	switch (State)
	{
	case EEnemyState::Passive:
	case EEnemyState::Investigating:
		GetCharacterMovement()->MaxWalkSpeed = PassiveSpeed;
		break;
	case EEnemyState::Combat:
		GetCharacterMovement()->MaxWalkSpeed = CombatSpeed;
		break;
	case EEnemyState::Dead:
	case EEnemyState::Frozen:
		GetCharacterMovement()->MaxWalkSpeed = DeadSpeed;
		break;
	default:
		break;
	}
}

void ASummonedWolf::ThrowObject_Implementation()
{
}

void ASummonedWolf::Howling_Implementation()
{
}

void ASummonedWolf::NormalAttack_Implementation()
{
	if (!HasAuthority())
	{
		return;
	}

	Multicast_PlayAttackMontage();
}

void ASummonedWolf::Multicast_PlayAttackMontage_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance || !AttackMontage)
	{
		if (HasAuthority())
		{
			OnAttackEnd.Broadcast();
		}
		return;
	}

	AnimInstance->Montage_Play(AttackMontage);

	if (HasAuthority())
	{
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ASummonedWolf::OnAttackMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
	}
}

void ASummonedWolf::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// [Refactor] 몽타주 종료 콜백: 유효성 체크
	if (!IsValid(this)) return;
	bIsAttacking = false;
	OnAttackEnd.Broadcast();
}

void ASummonedWolf::OnAttackHit(const FHitResult& HitResult)
{
	if (!HasAuthority())
	{
		return;
	}

	AActor* HitActor = HitResult.GetActor();
	if (!HitActor)
	{
		return;
	}

	// 보스(Owner) 또는 같은 소환 늑대끼리는 피해 무시
	// IsValid 체크: HitActor가 펜딩킬 상태일 수 있음
	if (!IsValid(HitActor) || HitActor == GetOwner() || HitActor->IsA(ASummonedWolf::StaticClass()))
	{
		return;
	}

	UGameplayStatics::ApplyDamage(HitActor, AttackDamage, GetController(), this, UDamageType::StaticClass());
}

void ASummonedWolf::HandleHPZero()
{
	// [Refactor] 델리게이트 콜백: 유효성 체크
	if (!IsValid(this))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Refactor] ASummonedWolf::HandleHPZero: Actor is invalid"));
		return;
	}
	IEnemyCommonInterface::Execute_Die(this);
}

float ASummonedWolf::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (DamageCauser && IsValid(DamageCauser) && DamageCauser->IsA(ASummonedWolf::StaticClass()))
	{
		return 0.f;
	}

	if (StatusComponent)
	{
		StatusComponent->DecreaseHP(ActualDamage);
	}

	return ActualDamage;
}

void ASummonedWolf::Die_Implementation()
{
	if (!HasAuthority())
	{
		return;
	}

	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	StopChaseLoop();
	GetWorldTimerManager().ClearTimer(AttackCooldownTimerHandle);
	ApplyDeadState();
	SetLifeSpan(2.5f);
}

void ASummonedWolf::OnRep_IsDead()
{
	if (bIsDead)
	{
		ApplyDeadState();
	}
}

void ASummonedWolf::ApplyDeadState()
{
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->GravityScale = 0.f;

	if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		AnimInstance->Montage_Stop(0.2f);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (DieSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DieSound, GetActorLocation());
	}
}

void ASummonedWolf::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASummonedWolf, bIsDead);
}

