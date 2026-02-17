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
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "AI/Enemy_Character/SummonedWolf.h"
#include "Character/MainPlayer.h"
#include "NavigationSystem.h"

AEnemyAIBoss::AEnemyAIBoss()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(84.f, 192.f);

	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -192.f));

	StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));
	AttackCollisionComponent = CreateDefaultSubobject<UAttackCollisionComponent>(TEXT("AttackCollisionComponent"));

	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	bReplicates = true;
	SetReplicateMovement(true);
}

void AEnemyAIBoss::StartBossCombat()
{
	if (bIsCombatActive) return;

	bIsCombatActive = true;

	if (AEnemyAIBossController* AIC = Cast<AEnemyAIBossController>(GetController()))
	{
		AIC->StartBehaviorTree();
	}

	OnBossCombatStart.Broadcast(this);
}

void AEnemyAIBoss::EndBossCombat()
{
	bIsCombatActive = false;
	OnBossCombatEnd.Broadcast();
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
	AActor* SelectedPoint = SelectSpawnPoint();
	if (!SelectedPoint)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("[Boss] No valid statue spawn point."));
		}
		return;
	}

	PendingSpawnPoint = SelectedPoint;
	SpawnRetryCount = 0;

	const FVector SpawnLocation = SelectedPoint->GetActorLocation();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Yellow,
			FString::Printf(TEXT("[Boss] Selected SpawnPoint: %s (X=%.1f Y=%.1f Z=%.1f)"),
				*SelectedPoint->GetName(), SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z));
	}
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
			Forewarning->OnForewarningResolved.AddUObject(this, &AEnemyAIBoss::OnForewarningResolved);
			return;
		}
	}

	TrySpawnStatueWithRetry();
}

void AEnemyAIBoss::OnForewarningComplete()
{
	// Legacy callback kept for compatibility.
}

void AEnemyAIBoss::OnForewarningResolved(bool bAreaClear)
{
	if (!HasAuthority())
	{
		return;
	}

	TrySpawnStatueWithRetry();
}

void AEnemyAIBoss::TrySpawnStatueWithRetry()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!PendingSpawnPoint.IsValid())
	{
		ClearSpawnState();
		return;
	}

	if (!StatueClass)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("[Boss] StatueClass is null."));
		}
		ClearSpawnState();
		return;
	}

	const FVector SpawnLocation = PendingSpawnPoint->GetActorLocation();

	if (IsSpawnAreaOccupied(SpawnLocation))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				1.5f,
				FColor::Orange,
				FString::Printf(TEXT("[Boss] Spawn area occupied. Retry %d/%d"),
					SpawnRetryCount + 1, MaxSpawnRetries));
		}
		if (SpawnRetryCount >= MaxSpawnRetries)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("[Boss] Statue spawn aborted (occupied area)."));
			}
			ClearSpawnState();
			return;
		}

		SpawnRetryCount++;
		GetWorldTimerManager().SetTimer(
			SpawnRetryTimerHandle,
			this,
			&AEnemyAIBoss::TrySpawnStatueWithRetry,
			SpawnRetryDelay,
			false
		);
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABossStatue* Spawned = GetWorld()->SpawnActor<ABossStatue>(
		StatueClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (Spawned)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.0f,
				FColor::Green,
				FString::Printf(TEXT("[Boss] Statue spawned at X=%.1f Y=%.1f Z=%.1f"),
					SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z));
		}
		ClearSpawnState();
		return;
	}

	if (SpawnRetryCount >= MaxSpawnRetries)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("[Boss] Statue spawn failed after retries."));
		}
		ClearSpawnState();
		return;
	}

	SpawnRetryCount++;
	GetWorldTimerManager().SetTimer(
		SpawnRetryTimerHandle,
		this,
		&AEnemyAIBoss::TrySpawnStatueWithRetry,
		SpawnRetryDelay,
		false
	);
}

bool AEnemyAIBoss::IsSpawnAreaOccupied(const FVector& Location) const
{
	if (!GetWorld())
	{
		return false;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjQuery;
	ObjQuery.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(StatueSpawnCheck), false, this);
	const bool bHit = GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		Location,
		FQuat::Identity,
		ObjQuery,
		FCollisionShape::MakeSphere(SpawnSafetyRadius),
		QueryParams
	);

	if (!bHit)
	{
		return false;
	}

	for (const FOverlapResult& OverlapResult : Overlaps)
	{
		const APawn* Pawn = Cast<APawn>(OverlapResult.GetActor());
		if (Pawn && Pawn->IsPlayerControlled())
		{
			return true;
		}
	}

	return false;
}

AActor* AEnemyAIBoss::SelectSpawnPoint()
{
	TArray<AActor*> ValidPoints;
	for (AActor* Point : StatueSpawnPoints)
	{
		if (IsValid(Point))
		{
			ValidPoints.Add(Point);
		}
	}

	if (ValidPoints.Num() > 0)
	{
		const int32 Index = SpawnPointCursor % ValidPoints.Num();
		SpawnPointCursor++;
		return ValidPoints[Index];
	}

	return IsValid(StatueSpawnPoint) ? StatueSpawnPoint : nullptr;
}

void AEnemyAIBoss::ClearSpawnState()
{
	GetWorldTimerManager().ClearTimer(SpawnRetryTimerHandle);
	PendingSpawnPoint.Reset();
	SpawnRetryCount = 0;
}

void AEnemyAIBoss::CleanupSummonedWolves()
{
	AliveSummonedWolves.RemoveAll([](const TWeakObjectPtr<ASummonedWolf>& Wolf)
	{
		return !Wolf.IsValid() || Wolf->bIsDead;
	});
}

int32 AEnemyAIBoss::ComputeDesiredWolfSpawnCount() const
{
	int32 AlivePlayers = 0;
	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMainPlayer::StaticClass(), Players);
	for (AActor* Actor : Players)
	{
		const AMainPlayer* Player = Cast<AMainPlayer>(Actor);
		if (!Player || !Player->StatusComponent)
		{
			continue;
		}

		if (Player->StatusComponent->CurrentHP > 0.f)
		{
			AlivePlayers++;
		}
	}

	const int32 PhaseBonus = FMath::Max(CurrentPhase - 1, 0);
	const int32 PlayerBonus = FMath::Clamp(AlivePlayers - 1, 0, 3);
	const int32 Desired = BaseSummonWolfCount + PhaseBonus + PlayerBonus;
	return FMath::Clamp(Desired, MinSummonWolfCount, MaxSummonWolfCount);
}

bool AEnemyAIBoss::IsWolfSpawnBlocked(const FVector& Location) const
{
	if (!GetWorld())
	{
		return true;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjQuery;
	ObjQuery.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(WolfSpawnCheck), false, this);
	const bool bHit = GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		Location,
		FQuat::Identity,
		ObjQuery,
		FCollisionShape::MakeSphere(WolfSpawnBlockRadius),
		QueryParams
	);

	return bHit;
}

bool AEnemyAIBoss::FindWolfSpawnLocation(FVector& OutLocation) const
{
	if (!GetWorld())
	{
		return false;
	}

	if (bUseFixedWolfSpawnPoints)
	{
		TArray<AActor*> ValidPoints;
		for (AActor* Point : SummonedWolfSpawnPoints)
		{
			if (IsValid(Point))
			{
				ValidPoints.Add(Point);
			}
		}

		if (ValidPoints.Num() > 0)
		{
			const int32 Index = FMath::RandRange(0, ValidPoints.Num() - 1);
			const FVector Candidate = ValidPoints[Index]->GetActorLocation();
			if (!IsWolfSpawnBlocked(Candidate))
			{
				OutLocation = Candidate;
				return true;
			}
		}
	}

	const UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys)
	{
		return false;
	}

	for (int32 i = 0; i < WolfSpawnSearchAttempts; ++i)
	{
		FNavLocation NavLoc;
		if (!NavSys->GetRandomReachablePointInRadius(GetActorLocation(), WolfSpawnMaxRadius, NavLoc))
		{
			continue;
		}

		const float Dist = FVector::Dist2D(GetActorLocation(), NavLoc.Location);
		if (Dist < WolfSpawnMinRadius)
		{
			continue;
		}

		if (IsWolfSpawnBlocked(NavLoc.Location))
		{
			continue;
		}

		OutLocation = NavLoc.Location;
		return true;
	}

	return false;
}

void AEnemyAIBoss::SpawnWolvesSequence()
{
	if (!HasAuthority() || !SummonedWolfClass)
	{
		return;
	}

	CleanupSummonedWolves();

	const int32 AliveCount = AliveSummonedWolves.Num();
	const int32 Slots = FMath::Max(0, MaxAliveSummonedWolves - AliveCount);
	if (Slots <= 0)
	{
		return;
	}

	const int32 SpawnCount = FMath::Min(ComputeDesiredWolfSpawnCount(), Slots);
	for (int32 i = 0; i < SpawnCount; ++i)
	{
		FVector SpawnLocation = FVector::ZeroVector;
		if (!FindWolfSpawnLocation(SpawnLocation))
		{
			break;
		}

		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		ASummonedWolf* Spawned = GetWorld()->SpawnActor<ASummonedWolf>(
			SummonedWolfClass,
			SpawnLocation,
			FRotator::ZeroRotator,
			Params
		);

		if (Spawned)
		{
			AliveSummonedWolves.Add(Spawned);
		}
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

void AEnemyAIBoss::ExecuteSummonWolves()
{
	if (!HasAuthority()) return;

	SpawnWolvesSequence();
	Multicast_PlaySummonWolvesMontage();
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

void AEnemyAIBoss::Multicast_PlaySummonWolvesMontage_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	UAnimMontage* MontageToPlay = SummonWolvesMontage ? SummonWolvesMontage : SummonStatueMontage;

	if (AnimInstance && MontageToPlay)
	{
		AnimInstance->Montage_Play(MontageToPlay);

		if (HasAuthority())
		{
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &AEnemyAIBoss::OnSummonWolvesMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
		}
	}
	else if (HasAuthority())
	{
		OnSummonWolvesEnd.Broadcast();
	}
}

void AEnemyAIBoss::ExecuteThrust()
{
	if (!HasAuthority()) return;

	Multicast_PlayThrustMontage();
}

void AEnemyAIBoss::Multicast_PlayThrustMontage_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && ThrustMontage)
	{
		AnimInstance->Montage_Play(ThrustMontage);

		if (HasAuthority())
		{
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &AEnemyAIBoss::OnThrustMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, ThrustMontage);
		}
	}
}

void AEnemyAIBoss::ExecuteSpecialAttack()
{
	if (!HasAuthority()) return;

	Multicast_PlaySpecialAttackMontage();
}

void AEnemyAIBoss::Multicast_PlaySpecialAttackMontage_Implementation()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SpecialAttackMontage)
	{
		AnimInstance->Montage_Play(SpecialAttackMontage);

		if (HasAuthority())
		{
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &AEnemyAIBoss::OnSpecialAttackMontageEnded);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, SpecialAttackMontage);
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

void AEnemyAIBoss::OnSummonWolvesMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	OnSummonWolvesEnd.Broadcast();
}

void AEnemyAIBoss::OnThrustMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	OnThrustEnd.Broadcast();
}

void AEnemyAIBoss::OnSpecialAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	OnSpecialAttackEnd.Broadcast();
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
