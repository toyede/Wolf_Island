// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Enemy_Character/EnemyAIBoss.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/StatusComponent.h"
#include "Components/AttackCollisionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"
#include "Actors/BossStatue.h"
#include "Actors/StatueForewarning.h"
#include "BrainComponent.h"

AEnemyAIBoss::AEnemyAIBoss()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(84.f, 192.f);

	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -192.f));

	StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));
	AttackCollisionComponent = CreateDefaultSubobject<UAttackCollisionComponent>(TEXT("AttackCollisionComponent"));

	bReplicates = true;
	SetReplicateMovement(true);
}

void AEnemyAIBoss::BeginPlay()
{
	Super::BeginPlay();

	if (AttackCollisionComponent)
	{
		AttackCollisionComponent->OnHitActor.AddUObject(this, &AEnemyAIBoss::OnAttackHit);
		AttackCollisionComponent->AddIgnoredActor(this);
	}
}

void AEnemyAIBoss::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEnemyAIBoss, bIsDead);
	DOREPLIFETIME(AEnemyAIBoss, bIsRushing);
}

void AEnemyAIBoss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemyAIBoss::SpawnStatueSequence()
{
	FVector SpawnLocation = StatueSpawnPoint->GetActorLocation();

	if (ForewarningClass)
	{
		AStatueForewarning* Forewarning = GetWorld()->SpawnActor<AStatueForewarning>(
			ForewarningClass,
			SpawnLocation,
			FRotator::ZeroRotator
		);

		if (Forewarning)
		{
			Forewarning->OnForewarningComplete.AddUObject(this, &AEnemyAIBoss::OnForewarningComplete);
		}
	}
}

void AEnemyAIBoss::OnForewarningComplete()
{
	FVector SpawnLocation = StatueSpawnPoint->GetActorLocation();

	if (StatueClass)
	{
		GetWorld()->SpawnActor<ABossStatue>(
			StatueClass,
			SpawnLocation,
			FRotator::ZeroRotator
		);
	}
}

void AEnemyAIBoss::OnAttackHit(const FHitResult& HitResult)
{
	AActor* HitActor = HitResult.GetActor();
	if (!HitActor) return;

	FDamageEvent DamageEvent;
	HitActor->TakeDamage(CurrentDamage, DamageEvent, GetController(), this);
}

void AEnemyAIBoss::ExecuteAttack(int32 AttackIndex)
{
	if (!HasAuthority()) return;
	if (!AttackMontages.IsValidIndex(AttackIndex)) return;

	if (AttackDamages.IsValidIndex(AttackIndex))
	{
		CurrentDamage = AttackDamages[AttackIndex];
	}

	Multicast_PlayAttackMontage(AttackIndex);
}

void AEnemyAIBoss::Multicast_PlayAttackMontage_Implementation(int32 AttackIndex)
{
	if (!AttackMontages.IsValidIndex(AttackIndex)) return;

	if (AttackStartSockets.IsValidIndex(AttackIndex) && AttackEndSockets.IsValidIndex(AttackIndex))
	{
		AttackCollisionComponent->TraceStartSocketName = AttackStartSockets[AttackIndex];
		AttackCollisionComponent->TraceEndSocketName = AttackEndSockets[AttackIndex];
	}

	if (AttackRadiuses.IsValidIndex(AttackIndex))
	{
		AttackCollisionComponent->TraceRadius = AttackRadiuses[AttackIndex];
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	UAnimMontage* Montage = AttackMontages[AttackIndex];

	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);

		if (HasAuthority())
		{
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &AEnemyAIBoss::OnAttackMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
		}
	}
}

void AEnemyAIBoss::ExecuteRush()
{
	if (!HasAuthority()) return;

	bIsRushing = true;
	CurrentDamage = RushDamage;

	Multicast_PlayRushMontage();
}

void AEnemyAIBoss::Multicast_PlayRushMontage_Implementation()
{
	if (RushStartSocket != NAME_None)
	{
		AttackCollisionComponent->TraceStartSocketName = RushStartSocket;
	}
	if (RushEndSocket != NAME_None)
	{
		AttackCollisionComponent->TraceEndSocketName = RushEndSocket;
	}
	AttackCollisionComponent->TraceRadius = RushRadius;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && RushMontage)
	{
		AnimInstance->Montage_Play(RushMontage);

		if (HasAuthority())
		{
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &AEnemyAIBoss::OnRushMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, RushMontage);
		}
	}
}

void AEnemyAIBoss::ExecuteGroggy()
{
	if (!HasAuthority()) return;

	bIsRushing = false;

	Multicast_PlayGroggyMontage();

	GetWorldTimerManager().SetTimer(
		GroggyTimerHandle,
		this,
		&AEnemyAIBoss::EndGroggy,
		GroggyDuration,
		false
	);
}

void AEnemyAIBoss::Multicast_PlayGroggyMontage_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && GroggyMontage)
	{
		AnimInstance->Montage_Play(GroggyMontage);
	}
}

void AEnemyAIBoss::EndGroggy()
{
	Multicast_PlayGroggyGetUp();
}

void AEnemyAIBoss::Multicast_PlayGroggyGetUp_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && GroggyMontage)
	{
		AnimInstance->Montage_JumpToSection(TEXT("GetUp"), GroggyMontage);

		if (HasAuthority())
		{
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &AEnemyAIBoss::OnGroggyMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, GroggyMontage);
		}
	}
}

void AEnemyAIBoss::ExecuteSummonStatue()
{
	if (!HasAuthority()) return;

	SpawnStatueSequence();
	Multicast_PlaySummonMontage();
}

void AEnemyAIBoss::Multicast_PlaySummonMontage_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && SummonStatueMontage)
	{
		AnimInstance->Montage_Play(SummonStatueMontage);

		if (HasAuthority())
		{
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &AEnemyAIBoss::OnSummonStatueMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, SummonStatueMontage);
		}
	}
}

void AEnemyAIBoss::Multicast_StopMontage_Implementation(float BlendOut)
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Stop(BlendOut);
	}
}

void AEnemyAIBoss::Die_Implementation()
{
	if (!HasAuthority()) return;
	if (bIsDead) return;

	bIsDead = true;
	ApplyDeadState();

	if (AEnemyAIBossController* AIC = Cast<AEnemyAIBossController>(GetController()))
	{
		AIC->GetBrainComponent()->StopLogic(TEXT("Boss Dead"));
	}

	SetLifeSpan(2.5f);
}

void AEnemyAIBoss::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	OnBossAttackEnd.Broadcast();
}

void AEnemyAIBoss::OnRushMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsRushing = false;
	OnBossRushEnd.Broadcast();
}

void AEnemyAIBoss::OnGroggyMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (AEnemyAIBossController* AIC = Cast<AEnemyAIBossController>(GetController()))
	{
		AIC->SetNewState(EBossState::Combat);
	}

	OnBossGroggyEnd.Broadcast();
}

void AEnemyAIBoss::OnSummonStatueMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	OnSummonStatueEnd.Broadcast();
}

void AEnemyAIBoss::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	if (!bIsRushing) return;

	if (OtherComp && OtherComp->GetCollisionObjectType() == ECC_GameTraceChannel1)
	{
		bIsRushing = false;

		if (AEnemyAIBossController* AIC = Cast<AEnemyAIBossController>(GetController()))
		{
			AIC->SetNewState(EBossState::Groggy);
		}

		if (ABossStatue* Statue = Cast<ABossStatue>(Other))
		{
			Statue->TakeDamage(RushDamage, FDamageEvent(), GetController(), this);
		}

		Multicast_StopMontage(0.2f);

		ExecuteGroggy();
	}
}

float AEnemyAIBoss::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (DamageCauser && DamageCauser->IsA<AEnemyAIBoss>())
	{
		return 0.f;
	}

	StatusComponent->DecreaseHP(ActualDamage);

	if (!bPhase2Triggered && StatusComponent->CurrentHP <= StatusComponent->MaxHP * 0.5f)
	{
		bPhase2Triggered = true;
		CurrentPhase = 2;
		OnPhaseChanged.Broadcast(CurrentPhase);
	}

	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Boss HP : %.0f"), StatusComponent->CurrentHP));

	if (StatusComponent->CurrentHP <= 0)
	{
		if (AEnemyAIBossController* AIC = Cast<AEnemyAIBossController>(GetController()))
		{
			AIC->SetNewState(EBossState::Dead);
		}
	}
	return ActualDamage;
}

void AEnemyAIBoss::OnRep_IsDead()
{
	if (bIsDead)
	{
		ApplyDeadState();
	}
}

void AEnemyAIBoss::ApplyDeadState()
{
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->GravityScale = 0.f;

	GetMesh()->GetAnimInstance()->Montage_Stop(0.2f);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (DieSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DieSound, GetActorLocation());
	}
}