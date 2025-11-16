// Fill out your copyright notice in the Description page of Project Settings.


#include "Moon/MoonlightInfectionSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Components/LightComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"

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

void AMoonlightInfectionSystem::CheckAllPlayers()
{
	// 서버에서만 실행 (멀티플레이 대비)
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	// 모든 BP_MainCharacter 찾기
	// 주의: BP_MainCharacter 클래스가 실제 프로젝트의 클래스 이름과 일치해야 함
	TArray<AActor*> Players;

	// 방법 1: 특정 클래스로 찾기 (프로젝트에 BP_MainCharacter C++ 클래스가 있는 경우)
	// UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABP_MainCharacter::StaticClass(), Players);

	// 방법 2: Character 클래스로 찾기 (모든 캐릭터)
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), Players);

	// 방법 3: Tag로 찾기 (더 유연함)
	// UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Player"), Players);

	// ===== 추가: 디버그 로그 =====
	if (bShowDebugMessages && Players.Num() == 0)
	{
		// 플레이어를 못 찾았을 때만 로그 출력 (스팸 방지)
		static bool bLoggedOnce = false;
		if (!bLoggedOnce)
		{
			UE_LOG(LogTemp, Warning, TEXT("[MoonlightSystem] No players found! GetAllActorsOfClass returned 0 characters."));
			bLoggedOnce = true;
		}
		return;
	}

	// 첫 체크 시에만 플레이어 수 로그
	static bool bFirstCheck = true;
	if (bFirstCheck && bShowDebugMessages)
	{
		UE_LOG(LogTemp, Log, TEXT("[MoonlightSystem] Found %d player(s)"), Players.Num());
		for (AActor* Player : Players)
		{
			UE_LOG(LogTemp, Log, TEXT("[MoonlightSystem]   - Player: %s (Class: %s)"),
				*Player->GetName(), *Player->GetClass()->GetName());
		}
		bFirstCheck = false;
	}

	// 각 플레이어 체크
	for (AActor* Player : Players)
	{
		if (IsPlayerExposedToMoonlight(Player))
		{
			ApplyInfection(Player, InfectionPerCheck);
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
	// TODO: 나중에 BP_MainCharacter에 InfectionLevel 변수 추가 후 구현
	// 지금은 프로토타입이므로 출력만

	// 임시: 간단한 로그 및 화면 출력
	FString PlayerName = Player->GetName();
	FString Message = FString::Printf(TEXT("Player: %s, Infection Up: %.1f"),
		*PlayerName, Amount);

	if (bShowDebugMessages)
	{
		UE_LOG(LogTemp, Warning, TEXT("[MoonlightSystem] %s"), *Message);
	}

	// 화면에 표시 (에디터/게임 플레이 중)
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, // Key (자동 생성)
			CheckInterval, // 표시 시간
			FColor::Red,
			Message,
			true, // 새로운 메시지 우선
			FVector2D(1.5f, 1.5f) // 텍스트 크기
		);
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


