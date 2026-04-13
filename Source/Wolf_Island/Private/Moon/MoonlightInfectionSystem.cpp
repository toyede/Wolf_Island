// Fill out your copyright notice in the Description page of Project Settings.


#include "Moon/MoonlightInfectionSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Components/LightComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "Actors/Interfaces/SkyInterface.h"
#include "Interaction/Repair_Actor.h"

#include "Character/MainPlayer.h"
#include "Components/StatusComponent.h"
#include "Character/WerewolfInfected.h"
#include "Character/MainPlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Actors/SpectatorCameraActor.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
AMoonlightInfectionSystem::AMoonlightInfectionSystem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AMoonlightInfectionSystem::BeginPlay()
{
	Super::BeginPlay();
	// BP_DynamicSky 찾기
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		// 클래스 이름으로 BP_DynamicSky 찾기
		if (Actor->GetName().Contains(TEXT("BP_DynamicSky")) ||
			Actor->GetClass()->GetName().Contains(TEXT("BP_DynamicSky")))
		{
			DynamicSkyActor = Actor;

			// MoonDirectionalLight 컴포넌트 찾기
			TArray<UActorComponent*> Components;
			Actor->GetComponents(ULightComponent::StaticClass(), Components);

			for (UActorComponent* Component : Components)
			{
				if (Component->GetName().Contains(TEXT("MoonDirectionalLight")) ||
					Component->GetName().Contains(TEXT("Moon")))
				{
					MoonLight = Cast<ULightComponent>(Component);
					if (bShowDebugMessages)
					{
						UE_LOG(LogTemp, Log, TEXT("[MoonlightSystem] MoonDirectionalLight found: %s"),
							*Component->GetName());
					}
					break;
				}
			}
			break;
		}
	}

	if (!MoonLight)
	{
		UE_LOG(LogTemp, Error, TEXT("[MoonlightSystem] MoonDirectionalLight not found! Check BP_DynamicSky component names."));
	}
	else if (bShowDebugMessages)
	{
		UE_LOG(LogTemp, Log, TEXT("[MoonlightSystem] System initialized successfully"));
	}

	// 감염 시작 이벤트 바인딩 (플레이어만)
	TArray<AActor*> PlayerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMainPlayer::StaticClass(), PlayerActors);
	BindPlayers(PlayerActors);
}

void AMoonlightInfectionSystem::ActivateInfectionCheck()
{
	if (!MoonLight)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MoonlightSystem] Cannot activate: MoonLight is null"));
		return;
	}

	if (NightlyTransformThreshold <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MoonlightSystem] NightlyTransformThreshold must be > 0. Current: %f"), NightlyTransformThreshold);
		return;
	}

	if (InfectionPerCheck <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MoonlightSystem] InfectionPerCheck must be > 0. Current: %f"), InfectionPerCheck);
		return;
	}

	if (GetWorldTimerManager().IsTimerActive(CheckTimerHandle))
	{
		UE_LOG(LogTemp, Warning, TEXT("[MoonlightSystem] Already active, skipping"));
		return;
	}

	// 외부에서 OnNightStarted 호출이 누락되어도, 매 활성화 시 이번 밤 누적값은 항상 0부터 시작한다.
	NightlyExposure.Empty();
	TriggeredThisNight.Empty();

	if (bShowDebugMessages)
	{
		UE_LOG(LogTemp, Log, TEXT("[MoonlightSystem] ACTIVATED - Check Interval: %.2fs"), CheckInterval);
		UE_LOG(LogTemp, Log, TEXT("[MoonlightSystem] SETTINGS - PerCheck: %.6f, Threshold: %.6f"), InfectionPerCheck, NightlyTransformThreshold);
	}

	// 타이머 시작 (CheckInterval 간격으로 CheckAllPlayers 반복 호출)
	GetWorldTimerManager().SetTimer(
		CheckTimerHandle,
		this,
		&AMoonlightInfectionSystem::CheckAllPlayers,
		CheckInterval,
		true // 반복
	);
}

void AMoonlightInfectionSystem::DeactivateInfectionCheck()
{
	if (bShowDebugMessages)
	{
		UE_LOG(LogTemp, Log, TEXT("[MoonlightSystem] DEACTIVATED"));
	}

	// 타이머 정지
	GetWorldTimerManager().ClearTimer(CheckTimerHandle);
}

void AMoonlightInfectionSystem::BindPlayers(const TArray<AActor*>& Players)
{
	for (AActor* Actor : Players)
	{
		if (AMainPlayer* Player = Cast<AMainPlayer>(Actor))
		{
			if (UStatusComponent* Status =
				Player->FindComponentByClass<UStatusComponent>())
			{
				Status->OnInfectionStarted.AddDynamic(
					this,
					&AMoonlightInfectionSystem::HandleInfectionStarted
				);
			}
		}
	}
}

void AMoonlightInfectionSystem::HandleInfectionStarted(UStatusComponent* StatusComp)
{
	if (!IsValid(StatusComp))
	{
		return;
	}

	if (!InfectedStatusList.Contains(StatusComp))
	{
		InfectedStatusList.Add(StatusComp);
	}
}

void AMoonlightInfectionSystem::StartSingleInfectionSequence(AMainPlayer* Player)
{
	if (!Player || GetNetMode() != NM_Standalone) return;

	// 아침으로 스킵
	if (IsValid(DynamicSkyActor) && DynamicSkyActor->Implements<USkyInterface>())
	{
		ISkyInterface::Execute_SkipToMorning(DynamicSkyActor);
	}

	// 수리된 것 중 랜덤 1개 파괴
	TArray<AActor*> RepairActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARepair_Actor::StaticClass(), RepairActors);
	for (AActor* Actor : RepairActors)
	{
		if (ARepair_Actor* RepairActor = Cast<ARepair_Actor>(Actor))
		{
			if (RepairActor->BreakRandomCompletedRepair()) break;
		}
	}

	// +20%는 OnDayStarted에서 처리
}

void AMoonlightInfectionSystem::OnNightStarted()
{
	// 밤 시작: 하루 누적량 초기화
	NightlyExposure.Empty();
	TriggeredThisNight.Empty();
}

void AMoonlightInfectionSystem::OnDayStarted()
{
	// 먼저 모든 늑대 세션 복원 (Possess 포함)
	if (GetNetMode() != NM_Standalone)
	{
		OnMorningStarted(); // 여기서 RestorePlayerAtDawn → Possess
	}

	// Possess 완료 후 +20% 적용 → OnRep이 정상적으로 클라이언트에 전파
	for (auto& WeakPlayer : TriggeredThisNight)
	{
		AMainPlayer* Player = WeakPlayer.Get();
		if (!IsValid(Player)) continue;

		if (Player->StatusComponent)
		{
			Player->StatusComponent->IncreaseInfectionBy(PostSequenceInfectionBonus);
		}
	}

	// 싱글에서는 위치 이동 + OnRespawn
	for (auto& WeakPlayer : TriggeredThisNight)
	{
		AMainPlayer* Player = WeakPlayer.Get();
		if (!IsValid(Player)) continue;

		if (GetNetMode() == NM_Standalone)
		{
			AActor* SpawnPoint = UGameplayStatics::GetActorOfClass(
				GetWorld(), APlayerStart::StaticClass());
			if (SpawnPoint)
			{
				Player->SetActorLocation(SpawnPoint->GetActorLocation());
			}
			Player->OnRespawn();
		}
	}

	NightlyExposure.Empty();
	TriggeredThisNight.Empty();
}

void AMoonlightInfectionSystem::StartMultiInfectionSequence(AMainPlayer* Player)
{
	if (!Player || !HasAuthority()) return;
	if (GetNetMode() == NM_Standalone) return; // 싱글이면 리턴

	APlayerController* PC = Cast<APlayerController>(Player->GetController());
	if (!PC) return;

	// 이미 변신 중이면 무시
	if (ActiveWerewolfSessions.Contains(PC)) return;

	FVector SpawnLocation = Player->GetActorLocation();
	FRotator SpawnRotation = Player->GetActorRotation();

	// 1) 원래 캐릭터 백업 + 숨김
	StoreOriginalCharacter(PC, Player);

	// 2) 늑대인간 스폰 + 빙의
	SpawnAndPossessWerewolf(PC, SpawnLocation);

	if (bShowDebugMessages)
	{
		UE_LOG(LogTemp, Log, TEXT("[MoonlightSystem] %s transformed into Werewolf (Multi)"),
			*Player->GetName());
	}
}

void AMoonlightInfectionSystem::OnMorningStarted()
{
	if (!HasAuthority()) return;

	// 모든 활성 늑대 세션 복귀 처리
	TArray<APlayerController*> PCsToRestore;
	for (auto& Pair : ActiveWerewolfSessions)
	{
		PCsToRestore.Add(Pair.Key);
	}

	for (APlayerController* PC : PCsToRestore)
	{
		RestorePlayerAtDawn(PC);
	}
}

void AMoonlightInfectionSystem::RestorePlayerAtDawn(APlayerController* PC)
{
	if (!PC) return;

	FWerewolfSessionData* Session = ActiveWerewolfSessions.Find(PC);
	if (!Session) return;

	AMainPlayer* OriginalPlayer = Session->OriginalCharacter.Get();
	ACharacter* Werewolf = Session->WerewolfCharacter.Get();

	if (!OriginalPlayer || !Werewolf) return;

	// 1) 늑대 위치 저장
	FVector RestoreLocation = Werewolf->GetActorLocation();
	FRotator RestoreRotation = Werewolf->GetActorRotation();

	// 2) 늑대에서 Unpossess
	PC->UnPossess();

	// 3) 원래 캐릭터를 늑대 위치로 이동
	OriginalPlayer->SetActorLocation(RestoreLocation);
	OriginalPlayer->SetActorRotation(RestoreRotation);

	// 4) 원래 캐릭터 복원
	OriginalPlayer->SetActorHiddenInGame(false);
	OriginalPlayer->SetActorEnableCollision(true);
	OriginalPlayer->SetActorTickEnabled(true);

	// 5) 원래 캐릭터에 다시 Possess
	PC->Possess(OriginalPlayer);

	// 복귀 시 관전 모드 해제
	if (AMainPlayerController* MPC = Cast<AMainPlayerController>(PC))
	{
		MPC->Client_ExitSpectateMode();
	}

	// 6) 늑대 제거
	Werewolf->Destroy();

	// 7) 세션 데이터 제거
	ActiveWerewolfSessions.Remove(PC);

	if (bShowDebugMessages)
	{
		UE_LOG(LogTemp, Log, TEXT("[MoonlightSystem] %s restored at dawn"),
			*OriginalPlayer->GetName());
	}
}

void AMoonlightInfectionSystem::Debug_ForceRestoreAll()
{
	OnMorningStarted();
}

void AMoonlightInfectionSystem::CheckAllPlayers()
{
	if (GetLocalRole() != ROLE_Authority) return;

	// 실제 사용 중인 런타임 값 확인용 로그 (이상 동작 시 이 값을 먼저 확인)
	UE_LOG(LogTemp, Warning, TEXT("[MoonlightSystem][TICK] InfectionPerCheck=%.6f | Threshold=%.4f"),
		InfectionPerCheck, NightlyTransformThreshold);

	for (int32 Index = InfectedStatusList.Num() - 1; Index >= 0; --Index)
	{
		UStatusComponent* StatusComp = InfectedStatusList[Index];
		if (!IsValid(StatusComp))
		{
			InfectedStatusList.RemoveAtSwap(Index);
			continue;
		}

		if (!StatusComp->IsInfected || StatusComp->bIsIncapacitated) continue;

		AActor* Player = StatusComp->GetOwner();
		if (!IsValid(Player)) continue;

		bool bIsCurrentlyWerewolf = false;
		for (auto& Pair : ActiveWerewolfSessions)
		{
			if (Pair.Value.OriginalCharacter == Player)
			{
				bIsCurrentlyWerewolf = true;
				break;
			}
		}
		if (bIsCurrentlyWerewolf) continue;

		if (IsPlayerExposedToMoonlight(Player))
		{
			if (AMainPlayer* MainPlayer = Cast<AMainPlayer>(Player))
			{
				if (TriggeredThisNight.Contains(MainPlayer)) continue;

				ApplyInfection(Player, InfectionPerCheck);

				float& Nightly = NightlyExposure.FindOrAdd(MainPlayer);
				Nightly += InfectionPerCheck;

				if (bShowDebugMessages)
				{
					float TotalInfection = StatusComp->CurrentInfectionRate;
					GEngine->AddOnScreenDebugMessage(-1, CheckInterval, FColor::Cyan,
						FString::Printf(TEXT("[%s] Total: %.1f%% | Nightly: %.1f%% / %.1f%%"),
							*MainPlayer->GetName(),
							TotalInfection,
							Nightly,
							NightlyTransformThreshold));
				}

				if (!TriggeredThisNight.Contains(MainPlayer) && Nightly >= NightlyTransformThreshold)
				{
					// 초과분 보정
					float Overflow = Nightly - NightlyTransformThreshold;
					if (Overflow > 0)
					{
						StatusComp->DecreaseInfection(Overflow);
					}

					TriggeredThisNight.Add(MainPlayer);

					if (GetNetMode() == NM_Standalone)
					{
						StartSingleInfectionSequence(MainPlayer);
					}
					else
					{
						StartMultiInfectionSequence(MainPlayer);
					}
				}
			}
		}
	}
}

bool AMoonlightInfectionSystem::IsPlayerExposedToMoonlight(AActor* Player)
{
	if (!Player || !MoonLight)
	{
		return false;
	}

	// 지면/장애물과 시작 지점이 겹치면 정지 상태에서만 차폐로 오검출될 수 있어, 라인트레이스로 단순화.
	FVector StartLocation = GetMoonlightCheckLocation(Player) + FVector(0.0f, 0.0f, 15.0f);

	// 달빛이 내려오는 방향의 반대 = 플레이어에서 달로
	FVector MoonDirection = MoonLight->GetForwardVector() * -1.0f;
	FVector EndLocation = StartLocation + (MoonDirection * TraceDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Player);
	QueryParams.bTraceComplex = false;

	// 정지 상태에서도 안정적으로 동작하도록 라인트레이스로 차폐 여부만 판정.
	FHitResult HitResult;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		QueryParams);

	// 디버그 드로우
	if (bDrawDebugTrace)
	{
		const FColor DebugColor = bHit ? FColor::Green : FColor::Red;

		DrawDebugLine(
			GetWorld(),
			StartLocation,
			bHit ? HitResult.Location : EndLocation,
			DebugColor,
			false,
			CheckInterval,
			0,
			1.0f
		);

		if (bHit)
		{
			DrawDebugPoint(
				GetWorld(),
				HitResult.Location,
				10.0f,
				FColor::Yellow,
				false,
				CheckInterval
			);
		}
	}

	// 막힌 게 없으면 노출됨
	return !bHit;
}

void AMoonlightInfectionSystem::ApplyInfection(AActor* Player, float Amount)
{
	if (!IsValid(Player))
	{
		return;
	}

	UStatusComponent* StatusComp = Player->FindComponentByClass<UStatusComponent>();
	if (!StatusComp)
	{
		if (bShowDebugMessages)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MoonlightSystem] %s has no StatusComponent!"), *Player->GetName());
		}
		return;
	}

	if (StatusComp->IsInfected && !StatusComp->bIsIncapacitated)
	{
		StatusComp->IncreaseInfectionBy(Amount);

		if (bShowDebugMessages)
		{
			UE_LOG(LogTemp, Log, TEXT("[MoonlightSystem] %s infection increased by %.3f (Total: %.2f%%)"),
				*Player->GetName(), Amount, StatusComp->CurrentInfectionRate * 100.0f);
		}
	}
}

FVector AMoonlightInfectionSystem::GetMoonlightCheckLocation(AActor* Player)
{
	// BP_MainCharacter에서 "MoonlightCheckPoint" SceneComponent 찾기
	USceneComponent* CheckPoint = nullptr;

	TArray<UActorComponent*> Components;
	Player->GetComponents(USceneComponent::StaticClass(), Components);

	for (UActorComponent* Component : Components)
	{
		if (Component->GetName().Contains(TEXT("MoonlightCheckPoint")))
		{
			CheckPoint = Cast<USceneComponent>(Component);
			break;
		}
	}

	// SceneComponent가 있으면 그 위치, 없으면 플레이어 위치 + 기본 오프셋
	if (CheckPoint)
	{
		return CheckPoint->GetComponentLocation();
	}
	else
	{
		// 기본: 플레이어 위치 + 위로 180cm (머리 높이)
		return Player->GetActorLocation() + FVector(0, 0, 180.0f);
	}
}

void AMoonlightInfectionSystem::SpawnAndPossessWerewolf(APlayerController* PC, FVector Location)
{
	if (!PC || !WerewolfClass) return;

	FWerewolfSessionData SessionData;
	SessionData.OriginalCharacter = Cast<AMainPlayer>(PC->GetPawn());
	SessionData.OwningPC = PC;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ACharacter* Werewolf = GetWorld()->SpawnActor<ACharacter>(
		WerewolfClass,
		Location,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!Werewolf) return;

	// 기존 캐릭터에서 Unpossess만 하고 늑대에는 Possess 안 함
	PC->UnPossess();


	// 관전 대상 설정
	SetSpectateTarget(PC);

	SessionData.WerewolfCharacter = Werewolf;
	ActiveWerewolfSessions.Add(PC, SessionData);

	if (AMainPlayerController* MPC = Cast<AMainPlayerController>(PC))
	{
		MPC->Client_EnterSpectateMode();
	}
}

void AMoonlightInfectionSystem::StoreOriginalCharacter(APlayerController* PC, AMainPlayer* Player)
{
	if (!PC || !Player) return;

	// 캐릭터 숨기기 + 충돌 비활성화
	Player->SetActorHiddenInGame(true);
	Player->SetActorEnableCollision(false);
	Player->SetActorTickEnabled(false);
}

void AMoonlightInfectionSystem::SetSpectateTarget(APlayerController* PC)
{
	if (!PC) return;

	TArray<AActor*> FoundPlayers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMainPlayer::StaticClass(), FoundPlayers);

	for (AActor* Actor : FoundPlayers)
	{
		AMainPlayer* MP = Cast<AMainPlayer>(Actor);
		if (!MP || MP->IsHidden() || MP->GetController() == PC) continue;

		// 타겟 플레이어만 알려줌
		if (AMainPlayerController* MPC = Cast<AMainPlayerController>(PC))
		{
			MPC->Client_SetSpectateTarget(MP);
		}
		return;
	}
}

void AMoonlightInfectionSystem::SwitchSpectateTarget(APlayerController* PC)
{
	if (!PC) return;

	TArray<AActor*> FoundPlayers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMainPlayer::StaticClass(), FoundPlayers);

	AActor* CurrentView = PC->GetViewTarget();

	TArray<AMainPlayer*> ValidTargets;
	for (AActor* Actor : FoundPlayers)
	{
		AMainPlayer* MP = Cast<AMainPlayer>(Actor);
		if (!MP || MP->IsHidden() || MP->GetController() == PC) continue;
		ValidTargets.Add(MP);
	}

	if (ValidTargets.Num() == 0) return;

	AActor* CurrentFollowingTarget = nullptr;

	if (ASpectatorCameraActor* CurrentCam = Cast<ASpectatorCameraActor>(PC->GetViewTarget()))
	{
		CurrentFollowingTarget = CurrentCam->TargetActor; // 카메라가 쫓고 있는 본체를 가져옴
	}

	int32 CurrentIndex = ValidTargets.IndexOfByKey(CurrentFollowingTarget);

	// 찾지 못했다면(처음이라면) 0번, 찾았다면 다음 인덱스로
	int32 NextIndex = (CurrentIndex + 1) % ValidTargets.Num();
	AMainPlayer* NextTarget = ValidTargets[NextIndex];

	if (AMainPlayerController* MPC = Cast<AMainPlayerController>(PC))
	{
		MPC->Client_SetSpectateTarget(NextTarget);
	}
}

void AMoonlightInfectionSystem::NotifyWerewolfDown(ACharacter* Werewolf)
{
	if (!Werewolf || !HasAuthority()) return;

	// 현재 세션 중 해당 늑대인간을 사용하는 세션 찾기
	APlayerController* TargetPC = nullptr;
	for (auto& Pair : ActiveWerewolfSessions)
	{
		if (Pair.Value.WerewolfCharacter == Werewolf)
		{
			TargetPC = Pair.Key;
			break;
		}
	}

	/*if (TargetPC)
	{
		FWerewolfSessionData& Session = ActiveWerewolfSessions[TargetPC];
		AMainPlayer* OriginalPlayer = Session.OriginalCharacter.Get();

		if (OriginalPlayer)
		{
			Werewolf->SetActorEnableCollision(false);

			FVector DeathLocation = Werewolf->GetActorLocation();
			FRotator DeathRotation = Werewolf->GetActorRotation();

			OriginalPlayer->SetActorLocationAndRotation(DeathLocation, DeathRotation, false, nullptr, ETeleportType::TeleportPhysics);

			OriginalPlayer->SetActorHiddenInGame(false);
			OriginalPlayer->SetActorEnableCollision(true);
			OriginalPlayer->SetActorTickEnabled(true);

			if (UCharacterMovementComponent* MoveComp = OriginalPlayer->GetCharacterMovement())
			{
				MoveComp->StopMovementImmediately();
			}

			OriginalPlayer->KnockOut();
			TargetPC->Possess(OriginalPlayer);

			if (AMainPlayerController* MPC = Cast<AMainPlayerController>(TargetPC))
			{
				MPC->Client_ExitSpectateMode();
			}

			Werewolf->Destroy();
		}
	}*/
}
