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

// 테스트용 헤더
#include "Character/MainPlayer.h"
#include "Components/StatusComponent.h"
#include "Character/WerewolfInfected.h"
#include "Character/MainPlayerController.h"

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

	if (bShowDebugMessages)
	{
		UE_LOG(LogTemp, Log, TEXT("[MoonlightSystem] ACTIVATED - Check Interval: %.2fs"), CheckInterval);
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
	if (bShowDebugMessages)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
			FString::Printf(TEXT("[DayStarted] Triggered count: %d, Bonus: +%.0f%%"),
				TriggeredThisNight.Num(), PostSequenceInfectionBonus * 100.f));
	}

	// 아침: 트리거된 플레이어에게 +20% 적용
	for (auto& WeakPlayer : TriggeredThisNight)
	{
		AMainPlayer* Player = WeakPlayer.Get();
		if (!IsValid(Player)) continue;

		if (Player->StatusComponent)
		{
			Player->StatusComponent->IncreaseInfectionBy(PostSequenceInfectionBonus);

			if (bShowDebugMessages)
			{
				UE_LOG(LogTemp, Log, TEXT("[MoonlightSystem] %s +%.0f%% infection at dawn (Total: %.2f%%)"),
					*Player->GetName(),
					PostSequenceInfectionBonus * 100.f,
					Player->StatusComponent->CurrentInfectionRate * 100.f);
			}
		}
	}

	// 멀티: 늑대 복귀
	if (GetNetMode() != NM_Standalone)
	{
		OnMorningStarted();
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

		if (IsPlayerExposedToMoonlight(Player))
		{
			// 총 감염량 증가
			ApplyInfection(Player, InfectionPerCheck);

			if (AMainPlayer* MainPlayer = Cast<AMainPlayer>(Player))
			{
				// 하루 밤 누적량 증가
				float& Nightly = NightlyExposure.FindOrAdd(MainPlayer);
				Nightly += InfectionPerCheck;

				// 디버그 출력
				if (bShowDebugMessages)
				{
					float TotalInfection = StatusComp->CurrentInfectionRate;
					GEngine->AddOnScreenDebugMessage(-1, CheckInterval, FColor::Cyan,
						FString::Printf(TEXT("[%s] Total: %.2f%% | Nightly: %.2f%% / %.2f%%"),
							*MainPlayer->GetName(),
							TotalInfection * 100.f,
							Nightly * 100.f,
							NightlyTransformThreshold * 100.f));
				}

				// 하루 누적이 임계치 넘고, 이번 밤에 아직 트리거 안 됐으면
				if (!TriggeredThisNight.Contains(MainPlayer) && Nightly >= NightlyTransformThreshold)
				{
					TriggeredThisNight.Add(MainPlayer);

					if (GetNetMode() == NM_Standalone)
					{
						StartSingleInfectionSequence(MainPlayer);
					}
					else
					{
						StartMultiInfectionSequence(MainPlayer);
					}
					// +20%는 OnDayStarted에서 적용하므로 여기선 안 함
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

	// 1. 플레이어 체크 위치 (머리 위)
	FVector StartLocation = GetMoonlightCheckLocation(Player);

	// 2. 달 방향 벡터 (달빛이 내려오는 방향의 반대 = 플레이어에서 달로)
	FVector MoonDirection = MoonLight->GetForwardVector() * -1.0f;
	FVector EndLocation = StartLocation + (MoonDirection * TraceDistance);

	// 3. 캡슐 트레이스 설정
	FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Player); // 자기 자신은 무시
	QueryParams.bTraceComplex = false; // 단순 콜리전 사용 (성능 최적화)

	// 4. 캡슐 트레이스 실행
	FHitResult HitResult;
	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		FQuat::Identity, // 회전 없음
		ECC_Visibility,  // Visibility 채널
		CapsuleShape,
		QueryParams
	);

	// 5. 디버그 드로우
	if (bDrawDebugTrace)
	{
		// 노출되면 빨강, 가려지면 초록
		FColor DebugColor = bHit ? FColor::Green : FColor::Red;

		// 캡슐 그리기
		DrawDebugCapsule(
			GetWorld(),
			StartLocation,
			CapsuleHalfHeight,
			CapsuleRadius,
			FQuat::Identity,
			DebugColor,
			false,
			CheckInterval, // 다음 체크까지 표시
			0,
			2.0f // 선 두께
		);

		// 레이 라인 그리기
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

		// Hit 지점에 포인트 그리기
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

	// 6. 막힌 게 없으면 노출됨
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

	// 늑대는 자체 AI로 동작
	if (AWerewolfInfected* Wolf = Cast<AWerewolfInfected>(Werewolf))
	{
		Wolf->StartAI();
	}

	// 관전 대상 설정
	SetSpectateTarget(PC);

	SessionData.WerewolfCharacter = Werewolf;
	ActiveWerewolfSessions.Add(PC, SessionData);
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

		if (PC->IsLocalController())
		{
			GetWorldTimerManager().SetTimerForNextTick([PC, MP]()
				{
					if (PC && MP)
					{
						PC->SetViewTargetWithBlend(MP, 0.2f);
						UE_LOG(LogTemp, Log, TEXT("[Spectate] SERVER View Fixed to %s"), *MP->GetName());
					}
				});
		}

		else
		{
			AMainPlayerController* MainPC = Cast<AMainPlayerController>(PC);
			if (MainPC)
			{
				MainPC->Client_SetViewTargetWithBlend(MP, 0.2f);
			}
		}

		return; // 한 명 찾았으면 종료
	}
}