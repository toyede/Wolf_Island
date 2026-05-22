// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Enemy_Character/EnemyAIBoss.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Components/StatusComponent.h"
#include "Components/AttackCollisionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
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
#include "Actors/PrayerAltar.h"
#include "Actors/PrayerStatue.h"
#include "Actors/PrayerForewarning.h"
#include "NiagaraFunctionLibrary.h"

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
	if (!HasAuthority()) return;
	if (bIsCombatActive) return;

	bIsCombatActive = true;

	if (AEnemyAIBossController* BossAIC = Cast<AEnemyAIBossController>(GetController()))
	{
		BossAIC->StartBehaviorTree();
	}

	OnCombatStarted(this);
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

	RefreshStatueSpawnPointsFromTag();
	
	if (StatusComponent)
	{
		StatusComponent->CurrentHP = StatusComponent->MaxHP;
	}
	
	if (!HasAuthority() && bIsCombatActive)
	{
		OnCombatStarted(this);
	}
}

void AEnemyAIBoss::RefreshStatueSpawnPointsFromTag()
{
	if (!GetWorld() || StatueSpawnTag.IsNone())
	{
		return;
	}

	if (!StatueSpawnPoints.IsEmpty())
	{
		return;
	}

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), StatueSpawnTag, Found);
	for (AActor* A : Found)
	{
		if (IsValid(A))
		{
			StatueSpawnPoints.Add(A);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[Boss] Statue spawn discovery tag='%s', found=%d"), *StatueSpawnTag.ToString(), StatueSpawnPoints.Num());
}

void AEnemyAIBoss::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 클라이언트에서 보스가 파괴될 때 bIsCombatActive 복제가 먼저 오지 못한 경우
	// OnRep_CombatActive 대신 EndPlay에서 직접 UI 정리
	if (!HasAuthority() && bIsCombatActive)
	{
		OnCombatEnded();
	}

	// [Refactor] 델리게이트 해제: AttackCollisionComponent OnHitActor 바인딩 대칭 해제
	if (AttackCollisionComponent)
	{
		AttackCollisionComponent->OnHitActor.RemoveAll(this);
	}

	// 타이머 클리어
	GetWorldTimerManager().ClearTimer(GroggyTimerHandle);
	GetWorldTimerManager().ClearTimer(SpawnRetryTimerHandle);
	GetWorldTimerManager().ClearAllTimersForObject(this);

	// 석상 제거
	TArray<AActor*> Statues;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABossStatue::StaticClass(), Statues);
	for (AActor* Statue : Statues)
	{
		Statue->Destroy();
	}

	// 전조 이펙트 제거
	TArray<AActor*> Forewarnings;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStatueForewarning::StaticClass(), Forewarnings);
	for (AActor* FW : Forewarnings)
	{
		FW->Destroy();
	}

	// 소환된 늑대 제거
	for (auto& WolfWeak : AliveSummonedWolves)
	{
		if (ASummonedWolf* Wolf = WolfWeak.Get())
		{
			Wolf->Destroy();
		}
	}
	AliveSummonedWolves.Empty();

	Super::EndPlay(EndPlayReason);
}

void AEnemyAIBoss::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEnemyAIBoss, bIsDead);
	DOREPLIFETIME(AEnemyAIBoss, bIsRushing);
	DOREPLIFETIME(AEnemyAIBoss, bIsCombatActive);
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
			//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("[Boss] No valid statue spawn point - set StatueSpawnPoints or StatueSpawnPoint in editor."));
		}
		return;
	}

	PendingSpawnPoint = SelectedPoint;
	SpawnRetryCount = 0;

	const FVector SpawnLocation = SelectedPoint->GetActorLocation();
	if (GEngine)
	{
		//GEngine->AddOnScreenDebugMessage(
		//	-1,
		//	2.0f,
		//	FColor::Yellow,
		//	FString::Printf(TEXT("[Boss] Selected SpawnPoint: %s (X=%.1f Y=%.1f Z=%.1f)"),
		//		*SelectedPoint->GetName(), SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z));
	}
	if (ForewarningClass)
	{
		AStatueForewarning* Forewarning = GetWorld()->SpawnActor<AStatueForewarning>(
			ForewarningClass,
			SpawnLocation,
			SelectedPoint->GetActorRotation()
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
			//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("[Boss] StatueClass is null."));
		}
		ClearSpawnState();
		return;
	}

	const FVector RawSpawnLocation = PendingSpawnPoint->GetActorLocation();

	// Z 180도 회전 적용
	const FRotator SpawnRotation = FRotator(
		PendingSpawnPoint->GetActorRotation().Pitch,
		PendingSpawnPoint->GetActorRotation().Yaw + 180.f,
		PendingSpawnPoint->GetActorRotation().Roll
	);

	// 바닥 스냅: 스폰 포인트 아래로 라인트레이스 → 조각상 하단이 바닥에 닿도록 Z 보정
	FVector SpawnLocation = RawSpawnLocation;
	{
		FHitResult GroundHit;
		const FVector TraceStart = RawSpawnLocation + FVector(0.f, 0.f, 100.f);
		const FVector TraceEnd   = RawSpawnLocation - FVector(0.f, 0.f, 500.f);
		FCollisionQueryParams GroundParams(SCENE_QUERY_STAT(StatueGroundSnap), false, this);

		if (GetWorld()->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_WorldStatic, GroundParams))
		{
			// StatueGroundOffset: 조각상 메시 피벗이 바닥 기준이면 0, 중앙 기준이면 메시 반높이
			SpawnLocation = FVector(RawSpawnLocation.X, RawSpawnLocation.Y,
				GroundHit.ImpactPoint.Z + StatueGroundOffset);
		}
	}

	if (IsSpawnAreaOccupied(SpawnLocation))
	{
		if (GEngine)
		{
			//GEngine->AddOnScreenDebugMessage(
			//	-1,
			//	1.5f,
			//	FColor::Orange,
			//	FString::Printf(TEXT("[Boss] Spawn area occupied. Retry %d/%d"),
			//		SpawnRetryCount + 1, MaxSpawnRetries));
		}
		if (SpawnRetryCount >= MaxSpawnRetries)
		{
			if (GEngine)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("[Boss] Statue spawn aborted (occupied area)."));
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
		SpawnRotation,
		SpawnParams
	);

	if (Spawned)
	{
		if (GEngine)
		{
			//GEngine->AddOnScreenDebugMessage(
			//	-1,
			//	2.0f,
			//	FColor::Green,
			//	FString::Printf(TEXT("[Boss] Statue spawned at X=%.1f Y=%.1f Z=%.1f"),
			//		SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z));
		}
		ClearSpawnState();
		return;
	}

	if (SpawnRetryCount >= MaxSpawnRetries)
	{
		if (GEngine)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("[Boss] Statue spawn failed after retries."));
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
		// 플레이어만 체크 (소환된 늑대끼리는 막지 않음)
		const APawn* Pawn = Cast<APawn>(OverlapResult.GetActor());
		if (Pawn && Pawn->IsPlayerControlled())
		{
			UE_LOG(LogTemp, Log, TEXT("[Boss] WolfSpawn blocked by player: %s"), *Pawn->GetName());
			return true;
		}
	}

	return false;
}

AActor* AEnemyAIBoss::SelectSpawnPoint()
{
	if (StatueSpawnPoints.IsEmpty())
	{
		RefreshStatueSpawnPointsFromTag();
	}

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

	if (!IsValid(StatueSpawnPoint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Boss] No statue spawn point set. Tag='%s', array=%d"), *StatueSpawnTag.ToString(), StatueSpawnPoints.Num());
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
		UE_LOG(LogTemp, Warning, TEXT("[Boss] SpawnWolvesSequence: HasAuthority=%d, SummonedWolfClass=%d"),
			HasAuthority(), SummonedWolfClass != nullptr);
		return;
	}

	CleanupSummonedWolves();

	const int32 AliveCount = AliveSummonedWolves.Num();
	const int32 Slots = FMath::Max(0, MaxAliveSummonedWolves - AliveCount);
	if (Slots <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Boss] SpawnWolvesSequence: Slots=0, MaxAlive=%d, Alive=%d"), MaxAliveSummonedWolves, AliveCount);
		return;
	}

	const int32 SpawnCount = FMath::Min(ComputeDesiredWolfSpawnCount(), Slots);
	UE_LOG(LogTemp, Log, TEXT("[Boss] SpawnWolvesSequence: Trying to spawn %d wolves"), SpawnCount);

	for (int32 i = 0; i < SpawnCount; ++i)
	{
		FVector SpawnLocation = FVector::ZeroVector;
		if (!FindWolfSpawnLocation(SpawnLocation))
		{
			UE_LOG(LogTemp, Warning, TEXT("[Boss] SpawnWolvesSequence: FindWolfSpawnLocation failed for wolf %d"), i);
			break;
		}

		UE_LOG(LogTemp, Log, TEXT("[Boss] SpawnWolvesSequence: SpawnLocation[%d] = X=%.1f Y=%.1f Z=%.1f"), i, SpawnLocation.X, SpawnLocation.Y, SpawnLocation.Z);

		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ASummonedWolf* Spawned = GetWorld()->SpawnActor<ASummonedWolf>(
			SummonedWolfClass,
			SpawnLocation,
			FRotator::ZeroRotator,
			Params
		);

		if (Spawned)
		{
			UE_LOG(LogTemp, Log, TEXT("[Boss] SpawnWolvesSequence: Wolf %d spawned successfully"), i);
			AliveSummonedWolves.Add(Spawned);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[Boss] SpawnWolvesSequence: SpawnActor returned null for wolf %d"), i);
		}
	}
}

void AEnemyAIBoss::ExecuteSummonPrayer()
{
	if (!HasAuthority()) return;

	if (AEnemyAIBossController* AIC = Cast<AEnemyAIBossController>(GetController()))
	{
		AIC->SetNewState(EBossState::Prayer);
	}

	// 기존 늑대 제거
	for (auto& WolfWeak : AliveSummonedWolves)
	{
		if (ASummonedWolf* Wolf = WolfWeak.Get())
		{
			Wolf->Destroy();
		}
	}
	AliveSummonedWolves.Empty();

	// 다른 타이머 중단 (돌진, 늑대 소환 등)
	GetWorldTimerManager().ClearTimer(GroggyTimerHandle);
	GetWorldTimerManager().ClearTimer(SpawnRetryTimerHandle);

	// 애니메이션 중단 및 기도 애니메이션(임시) 재생
	Multicast_StopMontage(0.2f);
	Multicast_PlaySummonMontage();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("[Boss] Prayer Pattern Started - Invincible & Idle"));
	}

	// 2단계 액터 소환 로직은 여기서 호출될 예정
}

void AEnemyAIBoss::OnAttackHit(const FHitResult& HitResult)
{
	AActor* HitActor = HitResult.GetActor();
	if (!HitActor) return;

	FDamageEvent DamageEvent(UDamageType::StaticClass());
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

	// 사운드 재생
	if (AttackSounds.IsValidIndex(AttackIndex) && AttackSounds[AttackIndex])
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackSounds[AttackIndex], GetActorLocation());
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
	// 사운드 재생
	if (RushSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, RushSound, GetActorLocation());
	}

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
	// 사운드 재생
	if (GroggySound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, GroggySound, GetActorLocation());
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && GroggyMontage)
	{
		AnimInstance->Montage_Play(GroggyMontage);
	}
}

void AEnemyAIBoss::EndGroggy()
{
	// [Refactor] 타이머 콜백: 액터 및 World 유효성 체크
	if (!IsValid(this) || !GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Refactor] AEnemyAIBoss::EndGroggy: Actor or World is invalid"));
		return;
	}
	Multicast_PlayGroggyGetUp();
}

void AEnemyAIBoss::Multicast_PlayGroggyGetUp_Implementation()
{
	// 사운드 재생
	if (GroggyGetUpSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, GroggyGetUpSound, GetActorLocation());
	}

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
	// 사운드 재생
	if (SummonStatueSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SummonStatueSound, GetActorLocation());
	}

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
	// 사운드 재생
	if (SummonWolvesSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SummonWolvesSound, GetActorLocation());
	}

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

void AEnemyAIBoss::OnThrustImpact()
{
	// 서버에서만 판정 처리 — 클라이언트 노티파이 호출은 무시
	if (!HasAuthority()) return;

	const FVector Origin = GetActorLocation();

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);

	TArray<AActor*> FoundActors;
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Origin,
		ThrustRange,
		ObjectTypes,
		ACharacter::StaticClass(),
		IgnoreActors,
		FoundActors
	);

	for (AActor* Actor : FoundActors)
	{
		ACharacter* Target = Cast<ACharacter>(Actor);
		if (!Target) continue;

		// 데미지
		if (ThrustImpactDamage > 0.f)
		{
			FDamageEvent DamageEvent;
			Target->TakeDamage(ThrustImpactDamage, DamageEvent, GetController(), this);
		}

		// 넉백 방향: 보스 → 타겟 (수평) + 상방
		const FVector ToTarget = (Target->GetActorLocation() - Origin).GetSafeNormal2D();
		const FVector LaunchVelocity = ToTarget * ThrustForce + FVector(0.f, 0.f, UpwardForce);
		Target->LaunchCharacter(LaunchVelocity, true, true);
	}
}

void AEnemyAIBoss::Multicast_PlayThrustMontage_Implementation()
{
	// 사운드 재생
	if (ThrustSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ThrustSound, GetActorLocation());
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && ThrustMontage)
	{
		AnimInstance->Montage_Play(ThrustMontage, ThrustMontagePlayRate);

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
	// 사운드 재생
	if (SpecialAttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SpecialAttackSound, GetActorLocation());
	}

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

	// [Refactor] 널가드: BrainComponent null 체크 후 StopLogic 호출
	if (AEnemyAIBossController* AIC = Cast<AEnemyAIBossController>(GetController()))
	{
		if (AIC->GetBrainComponent())
		{
			AIC->GetBrainComponent()->StopLogic(TEXT("Boss Dead"));
		}
	}

	SetLifeSpan(2.5f);
}

void AEnemyAIBoss::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// [Refactor] 몽타주 종료 콜백: 유효성 체크
	if (!IsValid(this)) return;
	OnBossAttackEnd.Broadcast();
}

void AEnemyAIBoss::OnRushMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// [Refactor] 몽타주 종료 콜백: 유효성 체크
	if (!IsValid(this)) return;
	bIsRushing = false;
	OnBossRushEnd.Broadcast();
}

void AEnemyAIBoss::OnGroggyMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// [Refactor] 몽타주 종료 콜백: 유효성 체크
	if (!IsValid(this)) return;
	if (AEnemyAIBossController* AIC = Cast<AEnemyAIBossController>(GetController()))
	{
		AIC->SetNewState(EBossState::Combat);
	}

	OnBossGroggyEnd.Broadcast();
}

void AEnemyAIBoss::OnSummonStatueMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// [Refactor] 몽타주 종료 콜백: 유효성 체크
	if (!IsValid(this)) return;
	OnSummonStatueEnd.Broadcast();
}

void AEnemyAIBoss::OnSummonWolvesMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// [Refactor] 몽타주 종료 콜백: 유효성 체크
	if (!IsValid(this)) return;
	OnSummonWolvesEnd.Broadcast();
}

void AEnemyAIBoss::OnThrustMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// [Refactor] 몽타주 종료 콜백: 유효성 체크
	if (!IsValid(this)) return;
	OnThrustEnd.Broadcast();
}

void AEnemyAIBoss::OnSpecialAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// [Refactor] 몽타주 종료 콜백: 유효성 체크
	if (!IsValid(this)) return;
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
	if (AEnemyAIBossController* AIC = Cast<AEnemyAIBossController>(GetController()))
	{
		if (AIC->GetCurrentState() == EBossState::Prayer)
		{
			return 0.f;
		}
	}

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (DamageCauser && (DamageCauser->IsA<AEnemyAIBoss>() || DamageCauser->IsA<ASummonedWolf>()))
	{
		return 0.f;
	}

	StatusComponent->DecreaseHP(ActualDamage);

	// 피격 이펙트 — Unreliable Multicast (cosmetic)
	const FVector HitLocation = DamageCauser ? DamageCauser->GetActorLocation() : GetActorLocation();
	const FVector HitNormal = (GetActorLocation() - HitLocation).GetSafeNormal();
	Multicast_PlayHitEffect(GetActorLocation(), HitNormal);

	if (!bPhase2Triggered && StatusComponent->CurrentHP <= StatusComponent->MaxHP * Phase2HPThreshold)
	{
		bPhase2Triggered = true;
		CurrentPhase = 2;
		OnPhaseChanged.Broadcast(CurrentPhase);
	}

	if (!bPhase3Triggered && StatusComponent->CurrentHP <= StatusComponent->MaxHP * Phase3HPThreshold)
	{
		bPhase3Triggered = true;
		CurrentPhase = 3;
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

void AEnemyAIBoss::OnRep_CombatActive()
{
	if (bIsCombatActive)
	{
		OnBossCombatStart.Broadcast(this);
		OnCombatStarted(this);
	}
	else
	{
		OnBossCombatEnd.Broadcast();
		OnCombatEnded();
	}
}

void AEnemyAIBoss::ApplyDeadState()
{
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->GravityScale = 0.f;

	// [Refactor] OnRep 콜백 경유 가능: AnimInstance 유효성 체크 후 Montage 중지
	if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
	{
		AnimInst->Montage_Stop(0.2f);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	OnBossCombatEnd.Broadcast();

	if (DieSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DieSound, GetActorLocation());
	}
}

void AEnemyAIBoss::Multicast_PlayHitEffect_Implementation(FVector HitLocation, FVector HitNormal)
{
	// 피격 사운드
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, HitLocation);
	}

	// --- Niagara 우선, 없으면 Cascade 폴백 ---
	if (HitEffect)
	{
		if (HitEffectSocketName != NAME_None && GetMesh())
		{
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				HitEffect,
				GetMesh(),
				HitEffectSocketName,
				FVector::ZeroVector,
				HitNormal.Rotation(),
				EAttachLocation::KeepRelativeOffset,
				true
			);
		}
		else
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				HitEffect,
				HitLocation,
				HitNormal.Rotation()
			);
		}
	}
	else if (HitEffectCascade)
	{
		if (HitEffectSocketName != NAME_None && GetMesh())
		{
			UGameplayStatics::SpawnEmitterAttached(
				HitEffectCascade,
				GetMesh(),
				HitEffectSocketName,
				FVector::ZeroVector,
				HitNormal.Rotation(),
				EAttachLocation::KeepRelativeOffset
			);
		}
		else
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				HitEffectCascade,
				HitLocation,
				HitNormal.Rotation()
			);
		}
	}
}

