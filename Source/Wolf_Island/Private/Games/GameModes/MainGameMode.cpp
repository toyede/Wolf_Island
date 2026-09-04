// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/GameModes/MainGameMode.h"

#include "EngineUtils.h"
#include "Actors/BossEntryPlayerStart.h"
#include "Character/MainPlayer.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/StatusComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "Games/MainGameInstance.h"
#include "Games/MainGameState.h"
#include "Games/MainPlayerState.h"
#include "Games/MainSaveGame.h"
#include "Games/SaveInterface.h"
#include "Games/GameModes/MultiGameMode.h"
#include "Interaction/PortalActor.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "AI/Enemy_Character/EnemyAIBoss.h"
#include "Moon/MoonlightInfectionSystem.h" //KSH-아침 롤백 시 야간 상태 정리용
#include "AI/Animal/AnimalBase.h"
#include "Actors/AnimalSpawnPoint.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EngineUtils.h"

void AMainGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	MainGameInstance = Cast<UMainGameInstance>(GetGameInstance());
	PlayersSaveData.Empty();
	//메인 메뉴에서 입장 시 슬롯에서 불러온 세이브 게임 데이터 로드
	//저장된 플레이어 데이터도 세팅
	if (MainGameInstance && MainGameInstance->CurrenSaveGame)
	{
		CurrentSaveData = MainGameInstance->CurrenSaveGame;
		PlayersSaveData = CurrentSaveData->Players;
		UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] LOAD SAVE DATA FROM %s"), *CurrentSaveData->SlotName);
		CurrentSaveData->PrintSaveInfo();
	}
	//에디터에서 월드로 바로 입장 시 테스트 세이브 게임 데이터 생성 및 로드
	else
	{
		FString Type = Cast<AMultiGameMode>(this) ? "M" : "S";
		//테스트 세이브 게임 데이터가 있으면 불러오기
		if (UGameplayStatics::DoesSaveGameExist(Type+TEXT("TEST001_")+MapName,0))
		{
			UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] LOAD TEST SAVE GAME"));
			CurrentSaveData = Cast<UMainSaveGame>(UGameplayStatics::LoadGameFromSlot(Type+TEXT("TEST001_")+MapName, 0));
			PlayersSaveData = CurrentSaveData->Players;
		}
		//테스트 세이브 게임 데이터가 없으면 생성
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] CREATE TEST SAVE GAME"));
			CurrentSaveData = Cast<UMainSaveGame>(UGameplayStatics::CreateSaveGameObject(UMainSaveGame::StaticClass()));
			CurrentSaveData->SlotName = Type+TEXT("TEST001_")+MapName;
			CurrentSaveData->WorldName = MapName;
			UGameplayStatics::SaveGameToSlot(CurrentSaveData, CurrentSaveData->SlotName, 0);
		}
	}
}

void AMainGameMode::StartPlay()
{
	Super::StartPlay();
	
	//게임 스테이트에 선택된 역할 세팅
	if (AMainGameState* GS = GetGameState<AMainGameState>())
	{
		GS->RefreshSelectedRoles();
		UE_LOG(LogTemp, Warning, TEXT("Refresh SelectedRoles"));
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Game State Detected"));
	}
	
	LoadCurrentSave();
	
	//자동 저장 타이머
	if (TurnOnAutoSave)
	{
		GetWorld()->GetTimerManager().SetTimer(
		AutoSaveTimer,
		this,
		&AMainGameMode::SaveWorld,
		AutoSaveInterval * 60.0f,
		true);
	}

	BindPortalDelegates();
}

void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 동물 스폰 서브시스템 초기화 및 시작
	if (UAnimalSpawnSubsystem* SpawnSubsystem = GetWorld()->GetSubsystem<UAnimalSpawnSubsystem>())
	{
		for (TActorIterator<AAnimalSpawnPoint> It(GetWorld()); It; ++It)
		{
			AAnimalSpawnPoint* SpawnPoint = *It;
			if (IsValid(SpawnPoint) && SpawnPoint->AnimalClass)
			{
				SpawnSubsystem->AddSpawnInfo(
					SpawnPoint->AnimalClass,
					SpawnPoint->GetActorLocation(),
					SpawnPoint->SpawnRadius,
					SpawnPoint->InnerRadius,
					SpawnPoint->MaxCount,
					SpawnPoint->RespawnDelay
				);
			}
		}
		
		SpawnSubsystem->StartSpawning();
	}
}

void AMainGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	UnbindPortalDelegates();
	ActiveBossByPortal.Empty();

	Super::EndPlay(EndPlayReason);
}

void AMainGameMode::BindPortalDelegates()
{
	if (!HasAuthority())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<APortalActor> It(World); It; ++It)
		{
			APortalActor* Portal = *It;
			if (!IsValid(Portal) || BoundPortals.Contains(Portal))
			{
				continue;
			}

			Portal->OnPortalTriggered.AddDynamic(this, &AMainGameMode::HandlePortalTriggered);
			BoundPortals.Add(Portal);
		}
	}
}

void AMainGameMode::UnbindPortalDelegates()
{
	for (const TWeakObjectPtr<APortalActor>& PortalPtr : BoundPortals)
	{
		if (APortalActor* Portal = PortalPtr.Get())
		{
			Portal->OnPortalTriggered.RemoveDynamic(this, &AMainGameMode::HandlePortalTriggered);
		}
	}

	BoundPortals.Empty();
}

void AMainGameMode::HandlePortalTriggered(APortalActor* TriggeredPortal)
{
	if (!HasAuthority() || !IsValid(TriggeredPortal))
	{
		return;
	}

	SpawnBossForPortal(TriggeredPortal, TriggeredPortal->GetLastTriggeredPartyMembers());
}

void AMainGameMode::SpawnBossForPortal(APortalActor* TriggeredPortal, const TArray<AMainPlayer*>& PartyMembers)
{
	if (!IsValid(TriggeredPortal) || !TriggeredPortal->BossClassToSpawn)
	{
		return;
	}

	if (TWeakObjectPtr<AEnemyAIBoss>* ExistingBossPtr = ActiveBossByPortal.Find(TriggeredPortal))
	{
		if (ExistingBossPtr->IsValid())
		{
			return;
		}

		ActiveBossByPortal.Remove(TriggeredPortal);
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FVector SpawnLocation;
	FRotator SpawnRotation;
	
	if (TriggeredPortal->BossSpawnPoint)
	{
		SpawnLocation = TriggeredPortal->BossSpawnPoint->GetActorLocation();
		SpawnRotation = TriggeredPortal->BossSpawnPoint->GetActorRotation();
	}
	else
	{
		SpawnLocation = TriggeredPortal->BossSpawnTransform.GetLocation();
		SpawnRotation = TriggeredPortal->BossSpawnTransform.GetRotation().Rotator();
	}
	AEnemyAIBoss* SpawnedBoss = GetWorld()->SpawnActor<AEnemyAIBoss>(
		TriggeredPortal->BossClassToSpawn,
		SpawnLocation,
		SpawnRotation,
		Params
	);

	if (!IsValid(SpawnedBoss))
	{
		return;
	}

	ActiveBossByPortal.Add(TriggeredPortal, SpawnedBoss);
	SpawnedBoss->OnDestroyed.AddDynamic(this, &AMainGameMode::HandleManagedBossDestroyed);

	if (IsValid(TriggeredPortal->ExitPortal))
	{
		SpawnedBoss->OnBossCombatEnd.AddDynamic(TriggeredPortal->ExitPortal, &APortalActor::OnBossDefeated);
	}
	else
	{
		//KSH-여기서 바인딩에 실패하면 보스를 잡아도 탈출 포탈이 영영 열리지 않는다.
		//(보스 처치 후 포탈 이동이 "될 때도 있고 안 될 때도 있는" 증상의 원인)
		//HandleManagedBossDestroyed에서 보완 처리하므로 여기서는 원인 추적용 로그만 남긴다.
		UE_LOG(LogTemp, Error,
			TEXT("[GAMEMODE] Boss spawned but ExitPortal is invalid on portal %s - will fall back on boss death"),
			*TriggeredPortal->GetName());
	}

	SpawnedBoss->BossParticipants.Reset();
	for (AMainPlayer* PartyMember : PartyMembers)
	{
		if (IsValid(PartyMember))
		{
			SpawnedBoss->BossParticipants.AddUnique(PartyMember);
		}
	}

	// 1순위: StatusComponent nullptr 방어
	if (!SpawnedBoss->StatusComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[BOSS SPAWN] StatusComponent is null on spawned boss! Aborting."));
		SpawnedBoss->Destroy();
		return;
	}

	// 2순위: PartyMembers가 0명이면 최소 HP 보장
	const int32 PartyCount = FMath::Max(SpawnedBoss->BossParticipants.Num(), 1);
	SpawnedBoss->StatusComponent->MaxHP = PartyCount * 250.0f;
	SpawnedBoss->StatusComponent->CurrentHP = SpawnedBoss->StatusComponent->MaxHP;
	SpawnedBoss->StartBossCombat();

	BossRef = SpawnedBoss;
}

void AMainGameMode::HandleManagedBossDestroyed(AActor* DestroyedActor)
{
	const AEnemyAIBoss* DestroyedBoss = Cast<AEnemyAIBoss>(DestroyedActor);
	if (!DestroyedBoss)
	{
		return;
	}

	for (auto It = ActiveBossByPortal.CreateIterator(); It; ++It)
	{
		if (It.Value().Get() == DestroyedBoss)
		{
			//KSH-보스 스폰 시점에 ExitPortal이 유효하지 않으면 OnBossCombatEnd 바인딩이 안 된다.
			//보스가 "사망" 상태로 파괴됐다면 여기서 탈출 포탈을 직접 열어 준다.
			//(전멸로 인한 FailBossCombat 경로는 bIsDead가 false이므로 열리지 않는다)
			if (DestroyedBoss->bIsDead)
			{
				if (APortalActor* EntryPortal = It.Key().Get())
				{
					if (IsValid(EntryPortal->ExitPortal))
					{
						EntryPortal->ExitPortal->OnBossDefeated();
						UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] Boss defeated - ExitPortal opened (fallback)"));
					}
					else
					{
						UE_LOG(LogTemp, Error,
							TEXT("[GAMEMODE] Boss defeated but ExitPortal is still invalid on portal %s"),
							*EntryPortal->GetName());
					}
				}
			}

			It.RemoveCurrent();
			break;
		}
	}
}

void AMainGameMode::FailBossCombat(AEnemyAIBoss* FailedBoss)
{
	if (!IsValid(FailedBoss)) return;

	// 어느 포탈에서 스폰된 보스인지 찾아서 ExitPortal 리셋
	for (auto& Pair : ActiveBossByPortal)
	{
		if (Pair.Value.Get() == FailedBoss)
		{
			if (APortalActor* EntryPortal = Pair.Key.Get())
			{
				if (IsValid(EntryPortal->ExitPortal))
				{
					// 탈출 포탈 bBossDefeated 리셋 → 포탈 닫힘
					EntryPortal->ExitPortal->ResetBossDefeated();
					UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] ExitPortal reset on boss fail"));
				}
			}
			break;
		}
	}

	// OnBossCombatEnd 델리게이트 제거 → EndBossCombat() 호출해도 포탈이 열리지 않음
	FailedBoss->OnBossCombatEnd.Clear();
	FailedBoss->EndBossCombat();
	FailedBoss->Destroy();
}

void AMainGameMode::PostLogin(APlayerController* NewPlayer)
{
	AMainPlayerState* PS = Cast<AMainPlayerState>(NewPlayer->PlayerState);
	if (!PS) return;
	
	FString PlayerID = PS->GetPersistantId();
	UE_LOG(LogTemp, Warning, TEXT("[Player Login] %s"), *PlayerID);
	
	if (PlayersSaveData.Contains(PlayerID))
	{
		FPlayerSaveData& PlayerSaveData = PlayersSaveData[PlayerID];
		PS->SetItemsData(PlayerSaveData.InventoryItems);
		PS->SetPlayerRole(PlayerSaveData.PlayerRole);
	}
	
	Super::PostLogin(NewPlayer);
}

void AMainGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	//Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	RestartPlayer(NewPlayer);
	AfterRestartPlayer(NewPlayer, false);
}

void AMainGameMode:: RestartPlayer(AController* NewPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] MAINGAMEMODE RestartPlayer"))

	//KSH-컨트롤러 널가드
	if (!IsValid(NewPlayer))
	{
		UE_LOG(LogTemp, Error, TEXT("[GAMEMODE] RestartPlayer: invalid controller"));
		return;
	}

	if (APawn* OldPawn = NewPlayer->GetPawn())
	{
		//KSH-사망 리스폰은 폰 자신의 콜스택(몽타주 콜백, 인터랙션 타이머, 공격 트레이스) 안에서
		//호출되는 경우가 많다. 여기서 즉시 Destroy하면 반환 후 파괴된 폰에 접근해 파탈 에러가 난다.
		//진행 중이던 동작을 먼저 끊고, 실제 파괴는 다음 프레임으로 미룬다.
		if (AMainPlayer* OldMainPlayer = Cast<AMainPlayer>(OldPawn))
		{
			OldMainPlayer->CancelAllActions();
		}

		NewPlayer->UnPossess();
		NewPlayer->SetPawn(nullptr);

		OldPawn->SetActorEnableCollision(false);
		OldPawn->SetActorHiddenInGame(true);
		OldPawn->SetLifeSpan(0.05f);
	}

	Super::RestartPlayer(NewPlayer);
}

void AMainGameMode::Logout(AController* Exiting)
{
	//로그아웃 시 플레이어 정보 저장
	AMainPlayerState* PS = Cast<AMainPlayerState>(Exiting->PlayerState);
	
	if (PS)
	{
		SavePlayer(PS);
	}
	
	Super::Logout(Exiting);
}

void AMainGameMode::AfterRestartPlayer(AController* Player, bool IsDead)
{
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] AfterRestartPlayer"))
	if (!Player)
	{
		return;
	}

	AMainPlayerState* PS = Cast<AMainPlayerState>(Player->PlayerState);
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] NO PLAYER STATE"))
		return;
	}
	if (!LoadPlayer(PS, IsDead))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] NO SAVE DETECTED ON AFTER RESTART. SAVE NEW PLAYER"))
		SavePlayer(PS);
	}
}

void AMainGameMode::SetActorCache()
{
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] SET ACTOR CACHE"));
	
	TArray<AActor*> SaveActors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), USaveInterface::StaticClass(), SaveActors);

	ActorCache.Empty();
	
	for (AActor* Actor : SaveActors)
	{
		if (ASavableActor* Savable = Cast<ASavableActor>(Actor))
		{
			ActorCache.FindOrAdd(Savable->GetGUID()) = Actor;
		}
	}
}

void AMainGameMode::SaveWorld()
{
	//JWY-월드 종료 중 autosave가 뒤늦게 실행되면 GameState/Actor 접근으로 크래시가 날 수 있어 저장을 건너뜀
	UWorld* World = GetWorld();
	if (!World || World->bIsTearingDown)
	{
		return;
	}

	UMainSaveGame* Save = CurrentSaveData;
	
	if (!Save)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] WORLD SAVE FAILED"));
		return;
	}
	
	//저장 가능 액터 데이터 저장
	//저장 인터페이스를 가진 모든 액터 가져오기
	TArray<AActor*> SaveActors;
	UGameplayStatics::GetAllActorsWithInterface(World, USaveInterface::StaticClass(), SaveActors);
	
	Save->SavedActors.Empty();

	// 레시피 저장 코드 실행 && 알 수 없는 기록 저장 코드 실행
	if (AMainGameState* GS = GetGameState<AMainGameState>())
{
	Save->SavedSharedRecipes = GS->SharedRecipes;
	Save->SavedUnlockedRecordIDs = GS->UnlockedRecordIDs;
}
	
	//각 액터의 저장 코드 실행
	for (AActor* Actor : SaveActors)
	{
		//JWY-월드 teardown 중 이미 파괴 중인 저장 대상 actor를 건너뛰어 종료 시점 크래시를 방어
		if (!IsValid(Actor))
		{
			continue;
		}

		if (ISaveInterface* Savable = Cast<ISaveInterface>(Actor))
		{
			FActorSaveData Data;
			Savable->Execute_SaveData(Actor, Data);
			UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] SAVE ACTOR [%s] | [%s]"), *Actor->GetName(), *Data.ActorID.ToString())
			//세이브 파일에 액터 저장 데이터 추가
			Save->SavedActors.FindOrAdd(Data.ActorID) = Data;
		}
	}	
	
	//폴리지 데이터 저장
	Save->RemovedFoliages.Append(RemovedFoliageData);
	RemovedFoliageData.Empty();
	
	//플레이어 데이터 저장
	SavePlayers();
	
	//세이브 파일에 플레이어 저장 데이터 추가
	Save->Players = PlayersSaveData;
	
	Save->SaveUnixTime = FDateTime::UtcNow().ToUnixTimestamp();
	
	//세이브 파일 슬롯에 저장
	UGameplayStatics::SaveGameToSlot(CurrentSaveData, CurrentSaveData->SlotName, 0);
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] TEST SAVE SLOT : %s"), *CurrentSaveData->SlotName);
	
	AMainGameState* GS = GetGameState<AMainGameState>();
	FChattingData Chat = FChattingData(
		TEXT("SYSTEM"),TEXT("저장 완료"), EMessageType::NOTICE);
	//GS->AddChattingMessage(Chat);
}

void AMainGameMode::LoadWorld()
{
	if (!HasAuthority()) return;
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] LOAD SLOT : %s"), *CurrentSaveData->SlotName);
	
	UMainSaveGame* Save =
		Cast<UMainSaveGame>(UGameplayStatics::LoadGameFromSlot(CurrentSaveData->SlotName, 0));

	if (!Save) return;
	
	LoadWorldFromSave(Save);
	
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] LOAD WORLD COMPLETE"));
}

void AMainGameMode::LoadWorldFromSave(UMainSaveGame* Save)
{
	if (!HasAuthority()) return;
	
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] LOAD WORLD FROM %s"), *CurrentSaveData->SlotName);
	
	//저장된 액터가 있으면 액터 로드
	if (!Save->SavedActors.IsEmpty() || Save->SavedActors.Num() != 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] LOAD SAVED ACTORS"));
		//초기 상태 액터 캐시 생성
		SetActorCache();
	
		//삭제된 기존 액터는 삭제
		for (auto& Pair : ActorCache)
		{
			if (!Save->SavedActors.Contains(Pair.Key))
			{
				UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] DESTROY ACTOR : %s"), *Pair.Value->GetName());
				Pair.Value->Destroy();
			}
		}
	
		//저장된 액터 로드
		for (auto& Pair : Save->SavedActors)
		{
			FGuid GUID = Pair.Key;
			FActorSaveData& Data = Pair.Value;
		
			//저장된 액터가 이미 존재하는 거면
			if (ActorCache.Contains(GUID))
			{
				//데이터 로드
				if (ISaveInterface* Savable = Cast<ISaveInterface>(ActorCache[GUID]))
				{
					Savable->Execute_LoadData(ActorCache[GUID], Data);
				}
			}
			//저장된 액터가 삭제됐으면
			else
			{
				if (!Data.ActorClass)
				{
					UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] SKIP SPAWN: ActorClass is null for GUID %s"), *GUID.ToString());
					continue;
				}
				//새로 스폰
				AActor* NewActor = GetWorld()->SpawnActor<AActor>(
					Data.ActorClass,
					Data.Transform);

				//데이터 로드
				if (IsValid(NewActor))
				{
					if (ISaveInterface* Savable = Cast<ISaveInterface>(NewActor))
					{
						Savable->Execute_LoadData(NewActor, Data);
					}
				}
			}
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] LOAD FOLIAGES"));
	//폴리지 데이터 로드
	for (const FRemovedFoliageData& FoliageData : Save->RemovedFoliages)
	{
		//월드 액터 스캔
		for (TActorIterator<AActor> TargetActor(GetWorld()); TargetActor; ++TargetActor)
		{
			//해당 액터의 컴포넌트 확인
			TArray<UInstancedStaticMeshComponent*> Comps;
			TargetActor->GetComponents<UInstancedStaticMeshComponent>(Comps);
			
			//인스턴스 스태틱 메시 컴포넌트가 없으면 건너뛰기
			if (Comps.Num() == 0) continue;
			
			//컴포넌트가 있으면 체크
			for (UInstancedStaticMeshComponent* ISMC : Comps)
			{
				//저장된 데이터의 메쉬와 같지 않으면 건너뛰기
				if (ISMC->GetStaticMesh() != FoliageData.Mesh) continue;
				
				//컴포넌트의 폴리지 인스턴스 개수 가져오기
				int32 Count = ISMC->GetInstanceCount();
				
				//폴리지 인스턴스 하나씩 확인
				for (int32 i = Count - 1; i >= 0; --i)
				{
					//폴리지 인스턴스 트랜스폼 가져오기
					FTransform Transform;
					ISMC->GetInstanceTransform(i, Transform, true);
					
					//폴리지 위치가이 삭제된 폴리지 데이터와 같으면 삭제 후 다음으로
					if (FVector::DistSquared(
							Transform.GetLocation(),
							FoliageData.Location) < 4.0f)
					{
						ISMC->RemoveInstance(i);
						break;
					}
				}
			}
		}
	}

	// 공유 레시피 로드
	if (AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>())
	{
		// 레시피
		GS->SharedRecipes.Empty();
		GS->SharedRecipes = Save->SavedSharedRecipes;
		GS->OnSharedRecipesChanged.Broadcast();

		// 알 수 없는 기록
		GS->UnlockedRecordIDs.Empty();
		GS->UnlockedRecordIDs = Save->SavedUnlockedRecordIDs;
		GS->OnUnlockedRecordsChanged.Broadcast();
	}
}

void AMainGameMode::SavePlayer(AMainPlayerState* PlayerState)
{
	//JWY-Logout/월드 종료 중 PlayerState나 World가 이미 정리된 경우 저장 로직이 죽은 객체를 접근하지 않도록 방어
	UWorld* World = GetWorld();
	if (!IsValid(PlayerState) || !World || World->bIsTearingDown) return;
	
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] SAVE PLAYER [%s]"), *PlayerState->GetPersistantId());
	
	//해당 플레이어의 아이디를 키로하는 맵에 데이터를 저장.
	FString PlayerID = PlayerState->GetPersistantId();
	FPlayerSaveData& PlayerSaveData = PlayersSaveData.FindOrAdd(PlayerID);
	
	PlayerSaveData.PlayerID = PlayerID;
	PlayerSaveData.PlayerRole = PlayerState->GetPlayerRole();
	
	//플레이어가 인간인 상태에서 저장, 기절인 상태, 죽은 상태에서 저장 구분.
	//죽고 리스폰 시 Status 풀충전, 아이템 그대로. -> 아이템은 PlayerState에 유지.
	
	//인간이 아닌 상태에서도 트랜스폼이나 컨트롤 데이터는 저장
	ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerState->GetPawn());
	
	if (PlayerCharacter)
	{
		//보스전 중이라면? - 보스전 입구를 스폰 지점으로
		if (PlayerState->GetIsBossStage())
		{
			PlayerSaveData.Transform = GetBossStageEnterPoint(PlayerState->GetOwningController());
		}
		//보스전 아니면 마지막 위치를 스폰 지점으로
		else
		{
			PlayerSaveData.Transform = PlayerCharacter->GetActorTransform();
			PlayerSaveData.Velocity = PlayerCharacter->GetVelocity();
			PlayerSaveData.ControlRotation = PlayerCharacter->GetControlRotation();
		}
	}
	
	//플레이어가 인간인 상태에서 MainPlayer 액터 데이터를 저장.
	if (AMainPlayer* TargetPlayer = Cast<AMainPlayer>(PlayerCharacter))
	{
		FMemoryWriter PlayerWriter(PlayerSaveData.SubBinaryData1, true);
		FObjectAndNameAsStringProxyArchive PlayerArchive(PlayerWriter, true);
		PlayerArchive.ArIsSaveGame = true;
		TargetPlayer->Serialize(PlayerArchive);
		UE_LOG(LogTemp, Warning, TEXT("Serialize Character"));

		if (UCharacterMovementComponent* MoveComp = TargetPlayer->GetCharacterMovement())
		{
			FMemoryWriter MovementWriter(PlayerSaveData.MovementBinaryData, true);
			FObjectAndNameAsStringProxyArchive MovementArchive(MovementWriter, true);
			MovementArchive.ArIsSaveGame = false;
			MoveComp->Serialize(MovementArchive);
			UE_LOG(LogTemp, Warning, TEXT("Serialize Movement"));
		}

		if (TargetPlayer->InventoryComponent)
		{
			FMemoryWriter InventoryWriter(PlayerSaveData.InventoryBinaryData, true);
			FObjectAndNameAsStringProxyArchive InventoryArchive(InventoryWriter, true);
			InventoryArchive.ArIsSaveGame = true;
			TargetPlayer->InventoryComponent->Serialize(InventoryArchive);
			UE_LOG(LogTemp, Warning, TEXT("Serialize Inventory"));
			PlayerState->SetItemsData(TargetPlayer->InventoryComponent->GetInventory());
		}

		if (TargetPlayer->StatusComponent)
		{
			FMemoryWriter StatusWriter(PlayerSaveData.StatusBinaryData, true);
			FObjectAndNameAsStringProxyArchive StatusArchive(StatusWriter, true);
			StatusArchive.ArIsSaveGame = true;
			TargetPlayer->StatusComponent->Serialize(StatusArchive);
			UE_LOG(LogTemp, Warning, TEXT("Serialize Status"));
			// 구조체 기반 저장도 같이 유지 (바이너리 Serialize 디버깅/호환 보강용)
			PlayerSaveData.StatusData = TargetPlayer->StatusComponent->SaveStatus();
			PlayerSaveData.HasStatusData = true;
		}
	}
	
	//아이템 데이터는 플레이어 스테이트에 있는 것을 저장.
	//<?>인벤토리 업데이트 할때마다 플레이어 스테이트에 아이템 데이터가 저장됨.
	PlayerSaveData.InventoryItems = PlayerState->GetItems();

	//개인 레시피 저장
	PlayerSaveData.SavedPersonalRecipes = PlayerState->PersonalRecipes;
	
	//선택창 대기 중인 뉴비를 위한 선택한 역할 리스트 업데이트
	if (AMainGameState* GS = GetGameState<AMainGameState>())
	{
		GS->RefreshSelectedRoles();
		UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] SELECTED ROLES UPDATE"));
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] SAVE PLAYER COMPLETE [%s]"), *PlayerState->GetPersistantId());
}

void AMainGameMode::SavePlayers()
{
	//플레이어 데이터 저장
	AMainGameState* GS = GetGameState<AMainGameState>();
	if (!GS) return;
	//월드에 있는 플레이어 순회
	for (APlayerState* PS : GS->PlayerArray)
	{
		//유효하지 않은 플레이어 스테이트면 건너뛰기
		if (!IsValid(PS)) continue;
		
		//각 플레이어의 저장 코드 실행
		AMainPlayerState* MPS = Cast<AMainPlayerState>(PS);
		if (!MPS) continue;
		SavePlayer(MPS);
	}
}

bool AMainGameMode::LoadPlayer(AMainPlayerState* PlayerState, bool IsDead)
{
	//JWY-재접속/리스폰 중 PlayerState가 유효하지 않은 경우 이후 Pawn/컴포넌트 접근을 막기 위해 조기 반환
	if (!IsValid(PlayerState)) return false;

	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] LOAD PLAYER DATA"));
	
	ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerState->GetPawn());
	
	//해당 플레이어 스테이트를 가진 컨트롤러의 아이디로 조회.
	FString PlayerID = PlayerState->GetPersistantId();
	
	//저장된 플레이어 목록 중 해당 플레이어가 없으면 로드 False.
	if (!PlayersSaveData.Contains(PlayerID))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] %s HAS NO SAVE DATA"), *PlayerID);
		return false;
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] %s HAS SAVE DATA"), *PlayerID);
	}
	
	FPlayerSaveData& PlayerSaveData = PlayersSaveData[PlayerID];
	
	PlayerState->SetItemsData(PlayerSaveData.InventoryItems);
	
	PlayerState->PersonalRecipes.Empty();
	PlayerState->PersonalRecipes = PlayerSaveData.SavedPersonalRecipes;

	if (AMainPlayer* TargetPlayer = Cast<AMainPlayer>(PlayerCharacter))
	{
		for (const FName& RecipeID : TargetPlayer->DefaultRecipes)
		{
			if (RecipeID != NAME_None && RecipeID.ToString() != TEXT("None"))
			{
				PlayerState->UnlockPersonalRecipe(RecipeID);
			}
		}
	}
	
	if (PlayerCharacter && !IsDead)
	{
		UE_LOG(LogTemp, Warning, TEXT("LOAD PLAYER %s : Transform | %s"), *PlayerID, *PlayerSaveData.Transform.ToString())
		PlayerCharacter->SetActorTransform(PlayerSaveData.Transform);
		//JWY-종료/리스폰 경합 중 CharacterMovement가 없으면 Velocity 복원에서 null 접근하지 않도록 방어
		if (UCharacterMovementComponent* MoveComp = PlayerCharacter->GetCharacterMovement())
		{
			MoveComp->Velocity = PlayerSaveData.Velocity;
		}
		if (PlayerCharacter->GetController())
		{
			PlayerCharacter->GetController()->SetControlRotation(PlayerSaveData.ControlRotation);
		}
	}
	
	PlayerState->SetIsBossStage(false);
	
	if (AMainPlayer* TargetPlayer = Cast<AMainPlayer>(PlayerCharacter))
	{
		FMemoryReader PlayerReader(PlayerSaveData.SubBinaryData1, true);
		FObjectAndNameAsStringProxyArchive PlayerArchive(PlayerReader, true);
		PlayerArchive.ArIsSaveGame = true;
		TargetPlayer->Serialize(PlayerArchive);
		UE_LOG(LogTemp, Warning, TEXT("Deserialize Character"));
		
		//JWY-CharacterMovement가 이미 정리된 경우 이동 데이터 역직렬화가 크래시로 이어지지 않도록 방어
		if (UCharacterMovementComponent* MoveComp = TargetPlayer->GetCharacterMovement())
		{
			FMemoryReader MovementWriter(PlayerSaveData.MovementBinaryData, true);
			FObjectAndNameAsStringProxyArchive MovementArchive(MovementWriter, true);
			MovementArchive.ArIsSaveGame = false;
			MoveComp->Serialize(MovementArchive);
			UE_LOG(LogTemp, Warning, TEXT("Deserialize Movement"));
		}
		
		//JWY-인벤토리 컴포넌트가 정리된 상태에서는 저장 데이터 복원 시 null 접근하지 않도록 방어
		if (TargetPlayer->InventoryComponent)
		{
			FMemoryReader InventoryReader(PlayerSaveData.InventoryBinaryData, true);
			FObjectAndNameAsStringProxyArchive InventoryArchive(InventoryReader, true);
			InventoryArchive.ArIsSaveGame = true;
			TargetPlayer->InventoryComponent->Serialize(InventoryArchive);
			TargetPlayer->InventoryComponent->InventoryChanged();
			UE_LOG(LogTemp, Warning, TEXT("Deserialize Inventory"));
		}
	    
		//JWY-사망 복원 또는 StatusComponent 정리 상태에서는 상태 역직렬화로 죽은 컴포넌트를 접근하지 않도록 방어
		if (!IsDead && TargetPlayer->StatusComponent)
		{
			FMemoryReader StatusReader(PlayerSaveData.StatusBinaryData, true);
			FObjectAndNameAsStringProxyArchive StatusArchive(StatusReader, true);
			StatusArchive.ArIsSaveGame = true;
			TargetPlayer->StatusComponent->Serialize(StatusArchive);
			UE_LOG(LogTemp, Warning, TEXT("Deserialize Status"));
		
			if (PlayerSaveData.HasStatusData)
			{
				TargetPlayer->StatusComponent->LoadStatus(PlayerSaveData.StatusData);
				UE_LOG(LogTemp, Warning, TEXT("Deserialize Status (Struct Override)"));
			}
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] LOAD PLAYER COMPLETE [%s]"), *PlayerID);
	return true;
}

void AMainGameMode::LoadPlayers()
{
	AMainGameState* GS = GetGameState<AMainGameState>();
	
	if (!GS) return;
	
	for (auto Player : GS->PlayerArray)
	{
		AMainPlayerState* PS = Cast<AMainPlayerState>(Player);
		LoadPlayer(PS);
	}
}

bool AMainGameMode::HasSaveData(FString PlayerId)
{
	return PlayersSaveData.Find(PlayerId) != nullptr;
}

void AMainGameMode::RespawnPlayer(AController* Player)
{
	RestartPlayer(Player);
}

UMainSaveGame* AMainGameMode::DuplicateSaveData(UMainSaveGame* TargetSaveGame)
{
	if (!TargetSaveGame) return nullptr;

	TArray<uint8> BinaryData;
	
	FMemoryWriter Writer(BinaryData, true);
	FObjectAndNameAsStringProxyArchive WriterArchive(Writer, true);
	WriterArchive.ArIsSaveGame = true;
	TargetSaveGame->Serialize(WriterArchive);

	UMainSaveGame* NewSave =
		NewObject<UMainSaveGame>(GetTransientPackage(), TargetSaveGame->GetClass());

	FMemoryReader Reader(BinaryData, true);
	FObjectAndNameAsStringProxyArchive ReaderArchive(Reader, true);
	ReaderArchive.ArIsSaveGame = true;
	NewSave->Serialize(ReaderArchive);

	return NewSave;
}

void AMainGameMode::SaveMorningSaveData()
{
	if (!HasAuthority()) return;
	
	//월드를 저장하고, 그 저장된 걸 복제해서 아침 세이브데이터 변수로 저장.
	//CurrentSaveData 는 그 뒤로 자동 저장되어 계속 덮어씌워지고, MorningSaveData 변수는 아침 때로 유지.
	//라고 생각했었는데 생각해보니까 이러면 월드 종료하면 아침 데이터가 날아감...
	//저장 슬롯으로 접미사에 _morning을 갖는 슬롯을 만들어 저장해놓는 게 좋겠다.
	SaveWorld();
	
	UMainSaveGame* NewSave = DuplicateSaveData(CurrentSaveData);
	MorningSaveData = NewSave;
	MorningSaveData->SlotName += TEXT("_morning");
	
	AMainGameState* GS = GetGameState<AMainGameState>();
	
	//세이브 파일 슬롯에 저장(슬롯 이름에 _morning 붙여서)
	if (UGameplayStatics::SaveGameToSlot(MorningSaveData, MorningSaveData->SlotName, 0))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] Morning Data Saved in %s"), *MorningSaveData->SlotName);
		FChattingData Chat = FChattingData(
		TEXT("SYSTEM"),TEXT("아침 데이터 저장"), EMessageType::NOTICE);
		GS->AddChattingMessage(Chat);
	} else
	{
		FChattingData Chat = FChattingData(
		TEXT("SYSTEM"),TEXT("아침 데이터 저장 실패"), EMessageType::ALERT);
		GS->AddChattingMessage(Chat);
	}
}

void AMainGameMode::BackToMorning()
{
	if (!HasAuthority()) return;
	
	//아침 데이터 불러오기
	FString MorningSlot = CurrentSaveData->SlotName+TEXT("_morning");
	UMainSaveGame* Save =
		Cast<UMainSaveGame>(UGameplayStatics::LoadGameFromSlot(MorningSlot, 0));
	
	if (!Save) return;
	
	CurrentSaveData = Save;
	CurrentSaveData->SlotName.RemoveFromEnd(TEXT("_morning"));
	PlayersSaveData = Save->Players;
	
	Save->PrintSaveInfo();
	
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] LOAD MORNING SAVE : %s"), *MorningSlot);
	
	//1. 월드 로드
	LoadWorldFromSave(Save);

	//KSH-야간 상태(늑대인간 세션, 변신한 AI) 정리
	ResetNightState();

	//2. 플레이어 로드
	LoadPlayers();

	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] LOAD MORNING COMPLETE"));
}

//KSH-월드를 아침으로 되돌릴 때 달빛 감염 시스템의 야간 상태를 함께 정리한다
void AMainGameMode::ResetNightState()
{
	if (!HasAuthority()) return;

	AMoonlightInfectionSystem* System = Cast<AMoonlightInfectionSystem>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AMoonlightInfectionSystem::StaticClass()));

	if (!System)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] ResetNightState: MoonlightInfectionSystem not found in world"));
		return;
	}

	System->ResetNightStateForRollback();
}

void AMainGameMode::LoadCurrentSave()
{
	if (!HasAuthority() || !CurrentSaveData) return;
	
	LoadWorldFromSave(CurrentSaveData);
	LoadPlayers();
}

FTransform AMainGameMode::GetBossStageEnterPoint(AController* Controller)
{
	TArray<AActor*> Actors;
	
	//월드에서 보스전 입구 스폰 포인트 찾기
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABossEntryPlayerStart::StaticClass(), Actors);
	
	//없으면 기본 리스폰 포인트로
	if (Actors.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] NO ACTORS"));
		AActor* StartSpot = FindPlayerStart(Controller);
		if (!StartSpot)
		{
			UE_LOG(LogTemp, Error, TEXT("[GAMEMODE] FindPlayerStart returned null!"));
			return FTransform::Identity;
		}
		return StartSpot->GetActorTransform();
	}
	
	return Actors[FMath::RandRange(0, Actors.Num() - 1)]->GetActorTransform();
}

//싱글에서 죽었을 때
void AMainGameMode::HandlePlayerDeath(AController* DeadPlayerController)
{
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE][SINGLE] HANDLE PLAYER DEATH"));

	//KSH-널가드: 컨트롤러가 이미 정리 중일 수 있음
	if (!IsValid(DeadPlayerController))
	{
		UE_LOG(LogTemp, Error, TEXT("[GAMEMODE][SINGLE] HandlePlayerDeath: invalid controller"));
		return;
	}

	// 보스 참가자 명단에서 제거 및 블랙보드 타겟 클리어
	if (IsValid(BossRef) && DeadPlayerController->GetPawn())
	{
		AMainPlayer* DeadPlayer = Cast<AMainPlayer>(DeadPlayerController->GetPawn());
		BossRef->BossParticipants.Remove(DeadPlayer);

		// 보스 AI의 블랙보드에서 현재 죽은 플레이어를 타겟팅하고 있다면 클리어
		if (AAIController* AIC = Cast<AAIController>(BossRef->GetController()))
		{
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(TEXT("Target")));
				if (CurrentTarget == DeadPlayer)
				{
					BB->ClearValue(TEXT("Target"));
					UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] Clear Boss Target Blackboard"));
				}
			}
		}

		// 싱글은 참가자가 1명이므로 죽으면 곧바로 보스 전투 종료
		// (모든 참가자가 살아있지 않으면 보스 종료)
		bool bAnyAlive = false;
		for (AMainPlayer* Participant : BossRef->BossParticipants)
		{
			if (IsValid(Participant) && !Participant->IsHidden())
			{
				bAnyAlive = true;
				break;
			}
		}

		if (!bAnyAlive)
		{
			UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE][SINGLE] All boss participants dead. Ending boss combat."));

			// 참가자들의 IsBossStage 초기화
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
			AEnemyAIBoss* FailedBoss = BossRef;
			BossRef = nullptr;
			FailBossCombat(FailedBoss);
		}
	}

	//사망한 당일 아침으로 부활(아침으로 월드 롤백)
	//KSH-널가드: 세이브 데이터가 없으면 롤백 자체가 불가능
	if (!CurrentSaveData)
	{
		UE_LOG(LogTemp, Error, TEXT("[GAMEMODE][SINGLE] HandlePlayerDeath: CurrentSaveData is null. Skip morning rollback."));
		return;
	}

	//아침 데이터 슬롯 구하기
	FString MorningSlotName = CurrentSaveData->SlotName+TEXT("_morning");
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] SEARCH SLOT : %s"), *MorningSlotName);
	
	//아침 데이터 슬롯에서 데이터 가져오기
	if (UMainSaveGame* Save =
		Cast<UMainSaveGame>(UGameplayStatics::LoadGameFromSlot(MorningSlotName, 0)))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] LOAD MORNING SAVE : %s"), *MorningSlotName);
	
		//현재 세이브 파일을 아침 데이터로 교체
		UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] Replace PlayerSaveData to Morning Data"));
		CurrentSaveData = Save;
		CurrentSaveData->SlotName.RemoveFromEnd(TEXT("_morning"));
		PlayersSaveData = Save->Players;
		
		//아침 세이브로 월드 로드
		LoadWorldFromSave(Save);

		//KSH-세이브 롤백은 SaveInterface 액터만 복원하므로 야간에 스폰된 늑대인간/변신한 AI가 그대로 남는다.
		//아침으로 되돌린 시점에 야간 상태를 함께 정리한다.
		ResetNightState();

		//아침 세이브로 플레이어 로드
		RestartPlayer(DeadPlayerController);
		AfterRestartPlayer(DeadPlayerController, true);

		//사망 리스폰은 LoadPlayer에서 위치를 복원하지 않으므로(IsDead=true), 아침 세이브의 플레이어
		//위치로 명시적으로 배치한다. 이렇게 하지 않으면 FindPlayerStart 실패 시 원점(0,0,0)에 스폰된다.
		if (AMainPlayerState* RespawnPS = Cast<AMainPlayerState>(DeadPlayerController->PlayerState))
		{
			const FString RespawnID = RespawnPS->GetPersistantId();
			if (PlayersSaveData.Contains(RespawnID))
			{
				const FPlayerSaveData& MorningData = PlayersSaveData[RespawnID];
				if (APawn* RespawnPawn = DeadPlayerController->GetPawn())
				{
					RespawnPawn->SetActorTransform(MorningData.Transform, false, nullptr, ETeleportType::TeleportPhysics);
					DeadPlayerController->SetControlRotation(MorningData.ControlRotation);
					UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] RESPAWN AT MORNING LOC | %s"), *MorningData.Transform.GetLocation().ToString());
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] RESPAWN: NO MORNING DATA FOR %s (원점 스폰 위험)"), *RespawnID);
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] LOAD MORNING COMPLETE"));
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] NO MORNING SAVE EXIST"));
	}
}


