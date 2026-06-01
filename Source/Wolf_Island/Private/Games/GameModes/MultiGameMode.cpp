// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/GameModes/MultiGameMode.h"

#include "Character/MainPlayerController.h"
#include "Games/MainPlayerState.h"
#include "Widgets/RoleSelection/RoleButton.h"
#include "Moon/MoonlightInfectionSystem.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AI/Enemy_Character/EnemyAIBoss.h"
#include "Games/MainGameInstance.h"

AMultiGameMode::AMultiGameMode()
{
	bPauseable = false;
	bStartPlayersAsSpectators = true;
}

void AMultiGameMode::PostLogin(APlayerController* NewPlayer)
{
	//채팅 테스트하는 데 플레이어 이름이 너무 길어서 귀염뽀짝 짧은 이름으로 재설정 해주는 개발용 코드
	if (!NewPlayer) return;

	AMainPlayerState* PS = Cast<AMainPlayerState>(NewPlayer->PlayerState);
	if (!PS) return;

	static int32 Counter = 1;
	static const TArray<FString> Adjs = {
		TEXT("새까만"), TEXT("순백의"), TEXT("노란"), TEXT("파란"),
		TEXT("붉은"), TEXT("보랏빛"), TEXT("청록색"), TEXT("회색"),
		TEXT("푸른"), TEXT("흰"), TEXT("검은"), TEXT("남색"),
		TEXT("자홍색"), TEXT("고동색"), TEXT("회적색"), TEXT("담청색"),
		TEXT("동빛"), TEXT("녹색"), TEXT("군청색"), TEXT("하늘색"),
		TEXT("옥색"), TEXT("주홍빛"), TEXT("자색"), TEXT("보라색"),
	};
	static const TArray<FString> Nouns = {
		TEXT("여우"), TEXT("늑대"), TEXT("토끼"), TEXT("곰"),
		TEXT("고라니"), TEXT("멧돼지"), TEXT("개"), TEXT("고양이"),
		TEXT("닭"), TEXT("땃쥐"), TEXT("까마귀"), TEXT("사슴"),
		TEXT("코끼리"), TEXT("다람쥐"), TEXT("매"), TEXT("살쾡이"),
		TEXT("거북이"), TEXT("앵무새"), TEXT("너구리"), TEXT("오소리"),
		TEXT("기린"), TEXT("하마"), TEXT("코끼리"), TEXT("양")
	};

	int32 A = FMath::RandRange(0, Adjs.Num()-1);
	int32 N = FMath::RandRange(0, Nouns.Num()-1);
	
	FString NewName = Adjs[A]+" "+Nouns[N];
	
	//PS->SetPlayerName(NewName);
	
	//실제 스팀 기반 세션 온라인 환경에서 로그인 시 사용할 ID
	//테스트 환경에서 이걸 사용할 시 스팀 연동이 안되어있기 때문에 테스트 실행할 때마다 ID가 변경되어 테스트 용은 고정.
	//테스트용 플레이어 아이디
	FString PlayerID = PS->GetPersistantId();
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] %s LOGIN"), *PlayerID);
	
	if (PlayersSaveData.Contains(PlayerID))
	{
		FPlayerSaveData& PlayerSaveData = PlayersSaveData[PlayerID];
		PS->SetItemsData(PlayerSaveData.InventoryItems);
		PS->SetPlayerRole(PlayerSaveData.PlayerRole);
	}
	
	AMainGameState* GS = GetGameState<AMainGameState>();
	if (GS)
	{
		FChattingData Chat = FChattingData(
			TEXT("알림"), PS->GetPlayerName() + TEXT(" 님이 접속했습니다."), EMessageType::NOTICE);
		GS->AddChattingMessage(Chat);
	}
	
	Super::PostLogin(NewPlayer);
}

void AMultiGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	AMainPlayerController* NewMainPlayerController = Cast<AMainPlayerController>(NewPlayer);
	if (!NewMainPlayerController) return;

	AMainPlayerState* PlayerState = Cast<AMainPlayerState>(NewMainPlayerController->PlayerState);
	if (!PlayerState) return;
	
	UE_LOG(LogTemp, Warning, TEXT("Entered Players Role : %d"), PlayerState->GetPlayerRole())
	
	if (PlayerState->GetPlayerRole() == ECharacterRole::NONE) return;
	
	RestartPlayer(NewPlayer);
}

void AMultiGameMode::RestartPlayer(AController* NewPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("Call RestartPlayer on MultiGamemode"));
	Super::RestartPlayer(NewPlayer);
}

bool AMultiGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	AMainPlayerState* PlayerState = Cast<AMainPlayerState>(Player->PlayerState);
	if (!PlayerState) return false;

	UE_LOG(LogTemp, Warning, TEXT("Check Role on ShouldSpawn : %d"), PlayerState->GetPlayerRole());
	
	if (PlayerState->GetPlayerRole() == ECharacterRole::NONE) return false;
	
	UE_LOG(LogTemp, Warning, TEXT("Allow to Spawn"));
	return true;
}

//RestartPlayer 실행 시 호출되는 함수... 플레이어 폰 생성
APawn* AMultiGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE MULTI] SpawnFor EXECUTED : Set Spawn Transform"))
	
	AMainPlayerState* PS = Cast<AMainPlayerState>(NewPlayer->PlayerState);
	
	FRotator StartRotation(ForceInit);
	StartRotation.Yaw = StartSpot->GetActorRotation().Yaw;
	FVector StartLocation = StartSpot->GetActorLocation();
	FTransform SpawnTransform = FTransform(StartRotation, StartLocation);
	
	//저장된 정보가 있으면(플레이어의 마지막 위치가 있으면) 그 곳에 소환.
	if (PlayersSaveData.Find(PS->GetPersistantId()))
	{
		//SpawnTransform = PlayersSaveData[PS->GetPersistantId()].Transform;
	}
	
	return SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
}

APawn* AMultiGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer,
	const FTransform& SpawnTransform)
{
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE MULTI] SpawnAt EXECUTED : Spawn Pawn"))

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AMainPlayerState* PS = Cast<AMainPlayerState>(NewPlayer->PlayerState);
	if (!PS) return nullptr;

	int32 RoleIndex = static_cast<int32>(PS->GetPlayerRole());
	if (!PlayerRoleClassList.IsValidIndex(RoleIndex) || !PlayerRoleClassList[RoleIndex])
	{
		UE_LOG(LogTemp, Error, TEXT("[GAMEMODE MULTI] Invalid RoleIndex %d or null class in PlayerRoleClassList"), RoleIndex);
		return nullptr;
	}

	AMainPlayer* Player = GetWorld()->SpawnActor<AMainPlayer>(
		PlayerRoleClassList[RoleIndex],
		SpawnTransform,
		Params);

	return Player;
}

void AMultiGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	if (UMainGameInstance* GI = Cast<UMainGameInstance>(GetGameInstance()))
	{
		if (AMainGameState* GS = Cast<AMainGameState>(GetWorld()->GetGameState()))
		{
			GS->SetCurrentSessionCode(GI->GetSessionCode());
		}
	}
}

void AMultiGameMode::Logout(AController* Exiting)
{
	//JWY-클라이언트 연결 정리 중 Exiting이 이미 유효하지 않은 예외 상황에서 PlayerState 접근 크래시를 막기 위해 먼저 방어
	if (!IsValid(Exiting))
	{
		UE_LOG(LogTemp, Warning, TEXT("[MULTI GAMEMODE] Logout called with invalid controller"));
		return;
	}

	AMainPlayerState* PS = Cast<AMainPlayerState>(Exiting->PlayerState);

	AMainGameState* GS = GetGameState<AMainGameState>();
	if (PS && GS)
	{
		FChattingData Chat = FChattingData(
			TEXT("알림"), PS->GetPlayerName()+TEXT(" 님이 나갔습니다."), EMessageType::NOTICE);
		GS->AddChattingMessage(Chat);
	}

	AMoonlightInfectionSystem* InfectionSystem = nullptr;
	//JWY-월드 teardown 중 감염 시스템 조회가 죽은 World를 타지 않도록 World 유효성 확인 후 조회
	if (UWorld* World = GetWorld())
	{
		if (!World->bIsTearingDown)
		{
			InfectionSystem = Cast<AMoonlightInfectionSystem>(
				UGameplayStatics::GetActorOfClass(World, AMoonlightInfectionSystem::StaticClass()));
		}
	}

	if (IsValid(InfectionSystem))
	{
		InfectionSystem->HandlePlayerLogout(Exiting);
	}

	Super::Logout(Exiting);

	if (IsValid(InfectionSystem))
	{
		InfectionSystem->EvaluateAllPlayersInfectedAndSkipMorning();
	}
}

bool AMultiGameMode::CheckRoleAvailable(ECharacterRole NewRole) const
{
	AMainGameState* GS = GetGameState<AMainGameState>();
	//JWY-세션 종료/맵 전환 타이밍에 GameState가 없으면 역할 확인에서 null 접근하지 않도록 방어
	if (!GS) return false;

	for (auto Player : GS->PlayerArray)
	{
		AMainPlayerState* PS = Cast<AMainPlayerState>(Player);
		if (PS && PS->GetPlayerRole() == NewRole) return false;
	}
	return true;
}

UClass* AMultiGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	AMainPlayerState* PS = Cast<AMainPlayerState>(InController->PlayerState);

	if (!PS || PS->GetPlayerRole() == ECharacterRole::NONE) return nullptr;

	int32 RoleIndex = static_cast<int32>(PS->GetPlayerRole());
	if (!PlayerRoleClassList.IsValidIndex(RoleIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("[GAMEMODE MULTI] RoleIndex %d out of bounds in PlayerRoleClassList"), RoleIndex);
		return nullptr;
	}

	return PlayerRoleClassList[RoleIndex];
}

void AMultiGameMode::HandlePlayerDeath(AController* DeadPlayerController)
{
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE][MULTI] HANDLE PLAYER DEATH"));

	if (!IsValid(DeadPlayerController))
	{
		return;
	}

	APlayerController* DeadPC = Cast<APlayerController>(DeadPlayerController);
	if (!IsValid(DeadPC))
	{
		return;
	}

	// 늑대 세션 중인 플레이어는 위치를 강제 재배치하지 않는다.
	// (아침 복귀 시 RestorePlayerAtDawn에서 늑대 최종 위치로 복귀해야 함)
	if (AMoonlightInfectionSystem* InfectionSystem = Cast<AMoonlightInfectionSystem>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AMoonlightInfectionSystem::StaticClass())))
	{
		if (InfectionSystem->ActiveWerewolfSessions.Contains(DeadPC))
		{
			if (AMainPlayer* Player = Cast<AMainPlayer>(DeadPC->GetPawn()))
			{
				Player->OnRespawn();
			}

			UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE][MULTI] Werewolf session player death: skip StartSpot relocation"));
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Werewolf session player death: skip StartSpot relocation"));
			return;
		}
	}

	// 기절 안풀어줘서 죽어버린다면?
	// 1. 스탯 30씩만 준다. - 감염상태 유지.
	if (AMainPlayer* Player = Cast<AMainPlayer>(DeadPC->GetPawn()))
	{
		// 보스 참가자 명단에서 제거 및 블랙보드 타겟 클리어
		if (AMainGameMode* GM = Cast<AMainGameMode>(GetWorld()->GetAuthGameMode()))
		{
			if (IsValid(GM->BossRef))
			{
				GM->BossRef->BossParticipants.Remove(Player);

				// 보스 AI의 블랙보드에서 현재 죽은 플레이어를 타겟팅하고 있다면 클리어
				if (AAIController* AIC = Cast<AAIController>(GM->BossRef->GetController()))
				{
					if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
					{
						AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(TEXT("Target")));
						if (CurrentTarget == Player)
						{
							BB->ClearValue(TEXT("Target"));
							UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] Clear Boss Target Blackboard"));
						}
					}
				}

				bool bAnyAlive = false;
				for (AMainPlayer* Participant : GM->BossRef->BossParticipants)
				{
					if (IsValid(Participant) && !Participant->IsHidden())
					{
						bAnyAlive = true;
						break;
					}
				}

			if (!bAnyAlive)
				{
					UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE][MULTI] All boss participants dead. Ending boss combat."));

					// 모든 참가자의 IsBossStage 초기화 (재도전을 위해)
					if (AMainGameState* GS = GetGameState<AMainGameState>())
					{
						for (APlayerState* PS : GS->PlayerArray)
						{
							if (AMainPlayerState* MPS = Cast<AMainPlayerState>(PS))
							{
								MPS->SetIsBossStage(false);
							}
						}
					}

					// 실패 처리: ExitPortal 리셋 + 델리게이트 해제 후 보스 파괴
					AEnemyAIBoss* FailedBoss = GM->BossRef;
					GM->BossRef = nullptr;
					FailBossCombat(FailedBoss);
				}
			}
		}

		Player->OnRespawn();
	}

	// 2. 리스폰 지점으로 스폰
	// 보스 전 중 죽음 -> 보스전 입구에서 리스폰
	AMainPlayerState* PS = Cast<AMainPlayerState>(DeadPC->PlayerState);
	if (!IsValid(PS) || !IsValid(DeadPC->GetPawn()))
	{
		return;
	}

	if (PS->GetIsBossStage())
	{
		PS->SetIsBossStage(false);
		const FTransform SpawnTransform = GetBossStageEnterPoint(DeadPC);
		DeadPC->GetPawn()->SetActorTransform(SpawnTransform);
	}
	else
	{
		if (AActor* StartSpot = FindPlayerStart(DeadPC))
		{
			DeadPC->GetPawn()->SetActorTransform(StartSpot->GetActorTransform());
		}
	}
}
