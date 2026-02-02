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

		if (ABossStatue* Statue = Cast<ABossStatue>(Other))
		{
			// 배율 없이 기본 데미지만 전달
			Statue->TakeDamage(RushDamage, FDamageEvent(), GetController(), this);
		}

		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Stop(0.2f);
		}

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