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
	
}

void AEnemyAIBoss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemyAIBoss::ExecuteAttack(int32 AttackIndex)
{
	if (!AttackMontages.IsValidIndex(AttackIndex)) return;

	UAnimMontage* Montage = AttackMontages[AttackIndex];
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

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
	UAnimMontage* Montage = RushMontage;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	
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
	OnBossRushEnd.Broadcast();
}

void AEnemyAIBoss::OnGroggyMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	OnBossGroggyEnd.Broadcast();
}
