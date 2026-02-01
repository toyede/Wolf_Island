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

void AEnemyAIBoss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemyAIBoss::OnAttackHit(const FHitResult& HitResult)
{
	AActor* HitActor = HitResult.GetActor();
	if (!HitActor)
	{
		return;
	}

	FDamageEvent DamageEvent;
	HitActor->TakeDamage(CurrentDamage, DamageEvent, GetController(), this);
}

void AEnemyAIBoss::ExecuteAttack(int32 AttackIndex)
{
	if (!AttackMontages.IsValidIndex(AttackIndex)) return;

	UAnimMontage* Montage = AttackMontages[AttackIndex];
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AttackDamages.IsValidIndex(AttackIndex))
	{
		CurrentDamage = AttackDamages[AttackIndex];
	}

	if (AttackStartSockets.IsValidIndex(AttackIndex) && AttackEndSockets.IsValidIndex(AttackIndex))
	{
		AttackCollisionComponent->TraceStartSocketName = AttackStartSockets[AttackIndex];
		AttackCollisionComponent->TraceEndSocketName = AttackEndSockets[AttackIndex];
	}

	if (AttackRadiuses.IsValidIndex(AttackIndex))
	{
		AttackCollisionComponent->TraceRadius = AttackRadiuses[AttackIndex];
	}

	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AEnemyAIBoss::OnAttackMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
	}
}

void AEnemyAIBoss::ExecuteRush()
{
	bIsRushing = true;

	UAnimMontage* Montage = RushMontage;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	CurrentDamage = RushDamage;

	if (RushStartSocket != NAME_None)
	{
		AttackCollisionComponent->TraceStartSocketName = RushStartSocket;
	}
	if (RushEndSocket != NAME_None)
	{
		AttackCollisionComponent->TraceEndSocketName = RushEndSocket;
	}
	
	AttackCollisionComponent->TraceRadius = RushRadius;

	if (Montage && AnimInstance)
	{
		AnimInstance->Montage_Play(Montage);
		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AEnemyAIBoss::OnRushMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
	}
}

void AEnemyAIBoss::ExecuteGroggy()
{
	bIsRushing = false;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && GroggyMontage)
	{
		AnimInstance->Montage_Play(GroggyMontage);

		GetWorldTimerManager().SetTimer(
			GroggyTimerHandle,
			this,
			&AEnemyAIBoss::EndGroggy,
			GroggyDuration,
			false
		);
	}
}

void AEnemyAIBoss::EndGroggy()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && GroggyMontage)
	{
		AnimInstance->Montage_JumpToSection(TEXT("GetUp"), GroggyMontage);

		FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &AEnemyAIBoss::OnGroggyMontageEnded);
		AnimInstance->Montage_SetEndDelegate(EndDelegate, GroggyMontage);
	}
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

		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Stop(0.2f);
		}

		ExecuteGroggy();
	}
}