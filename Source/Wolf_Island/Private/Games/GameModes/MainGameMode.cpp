// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/GameModes/MainGameMode.h"

#include "EngineUtils.h"
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
#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

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
}

void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	
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
	if (NewPlayer->GetPawn())
	{
		NewPlayer->GetPawn()->Destroy();
		NewPlayer->SetPawn(nullptr);
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
	UMainSaveGame* Save = CurrentSaveData;
	
	if (!Save)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] WORLD SAVE FAILED"));
		return;
	}
	
	//저장 가능 액터 데이터 저장
	//저장 인터페이스를 가진 모든 액터 가져오기
	TArray<AActor*> SaveActors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), USaveInterface::StaticClass(), SaveActors);
	
	Save->SavedActors.Empty();

	// 레시피 저장 코드 실행
	if (AMainGameState* GS = GetGameState<AMainGameState>())
	{
		Save->SavedSharedRecipes = GS->SharedRecipes;
	}
	
	//각 액터의 저장 코드 실행
	for (AActor* Actor : SaveActors)
	{
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
	GS->AddChattingMessage(Chat);
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
				//새로 스폰
				AActor* NewActor = GetWorld()->SpawnActor<AActor>(
					Data.ActorClass,
					Data.Transform);
			
				//데이터 로드
				if (ISaveInterface* Savable = Cast<ISaveInterface>(NewActor))
				{
					Savable->Execute_LoadData(NewActor, Data);
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
		// 1. 공유 레시피 초기화
		GS->SharedRecipes.Empty();

		// 2. 세이브 데이터 로드
		GS->SharedRecipes = Save->SavedSharedRecipes;

		// 3. UI 갱신
		GS->OnSharedRecipesChanged.Broadcast();
	}
}

void AMainGameMode::SavePlayer(AMainPlayerState* PlayerState)
{
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
		PlayerSaveData.Transform = PlayerCharacter->GetActorTransform();
		PlayerSaveData.Velocity = PlayerCharacter->GetVelocity();
		PlayerSaveData.ControlRotation = PlayerCharacter->GetControlRotation();
	}
	
	//플레이어가 인간인 상태에서 MainPlayer 액터 데이터를 저장.
	if (AMainPlayer* TargetPlayer = Cast<AMainPlayer>(PlayerCharacter))
	{	
		FMemoryWriter PlayerWriter(PlayerSaveData.SubBinaryData1, true);
		FObjectAndNameAsStringProxyArchive PlayerArchive(PlayerWriter, true);
		PlayerArchive.ArIsSaveGame = true;
		TargetPlayer->Serialize(PlayerArchive);
		UE_LOG(LogTemp, Warning, TEXT("Serialize Character"));
		
		FMemoryWriter MovementWriter(PlayerSaveData.MovementBinaryData, true);
		FObjectAndNameAsStringProxyArchive MovementArchive(MovementWriter, true);
		MovementArchive.ArIsSaveGame = false;
		TargetPlayer->GetCharacterMovement()->Serialize(MovementArchive);
		UE_LOG(LogTemp, Warning, TEXT("Serialize Movement"));
		
		FMemoryWriter InventoryWriter(PlayerSaveData.InventoryBinaryData, true);
		FObjectAndNameAsStringProxyArchive InventoryArchive(InventoryWriter, true);
		InventoryArchive.ArIsSaveGame = true;
		TargetPlayer->InventoryComponent->Serialize(InventoryArchive);
		UE_LOG(LogTemp, Warning, TEXT("Serialize Inventory"));
		
		FMemoryWriter StatusWriter(PlayerSaveData.StatusBinaryData, true);
		FObjectAndNameAsStringProxyArchive StatusArchive(StatusWriter, true);
		StatusArchive.ArIsSaveGame = true;
		TargetPlayer->StatusComponent->Serialize(StatusArchive);
		UE_LOG(LogTemp, Warning, TEXT("Serialize Status"));
		
		// 구조체 기반 저장도 같이 유지 (바이너리 Serialize 디버깅/호환 보강용)
		PlayerSaveData.StatusData = TargetPlayer->StatusComponent->SaveStatus();
		PlayerSaveData.HasStatusData = true;
		
		PlayerState->SetItemsData(TargetPlayer->InventoryComponent->GetInventory());
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
	//월드에 있는 플레이어 순회
	for (APlayerState* PS : GS->PlayerArray)
	{
		//각 플레이어의 저장 코드 실행
		AMainPlayerState* MPS = Cast<AMainPlayerState>(PS);
		SavePlayer(MPS);
	}
}

bool AMainGameMode::LoadPlayer(AMainPlayerState* PlayerState, bool IsDead)
{
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
		PlayerCharacter->SetActorTransform(PlayerSaveData.Transform);
		PlayerCharacter->GetCharacterMovement()->Velocity = PlayerSaveData.Velocity;
		PlayerCharacter->GetController()->SetControlRotation(PlayerSaveData.ControlRotation);
	}
	
	if (AMainPlayer* TargetPlayer = Cast<AMainPlayer>(PlayerCharacter))
	{
		FMemoryReader PlayerReader(PlayerSaveData.SubBinaryData1, true);
		FObjectAndNameAsStringProxyArchive PlayerArchive(PlayerReader, true);
		PlayerArchive.ArIsSaveGame = true;
		TargetPlayer->Serialize(PlayerArchive);
		UE_LOG(LogTemp, Warning, TEXT("Deserialize Character"));
		
		FMemoryReader MovementWriter(PlayerSaveData.MovementBinaryData, true);
		FObjectAndNameAsStringProxyArchive MovementArchive(MovementWriter, true);
		MovementArchive.ArIsSaveGame = false;
		TargetPlayer->GetCharacterMovement()->Serialize(MovementArchive);
		UE_LOG(LogTemp, Warning, TEXT("Deserialize Movement"));
		
		FMemoryReader InventoryReader(PlayerSaveData.InventoryBinaryData, true);
		FObjectAndNameAsStringProxyArchive InventoryArchive(InventoryReader, true);
		InventoryArchive.ArIsSaveGame = true;
		TargetPlayer->InventoryComponent->Serialize(InventoryArchive);
		TargetPlayer->InventoryComponent->InventoryChanged();
		UE_LOG(LogTemp, Warning, TEXT("Deserialize Inventory"));
	
		if (!IsDead)
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
	
	//2. 플레이어 로드
	LoadPlayers();
	
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] LOAD MORNING COMPLETE"));
}

void AMainGameMode::LoadCurrentSave()
{
	if (!HasAuthority() || !CurrentSaveData) return;
	
	LoadWorldFromSave(CurrentSaveData);
	LoadPlayers();
}

//싱글에서 죽었을 때
void AMainGameMode::HandlePlayerDeath(AController* DeadPlayerController)
{
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE][SINGLE] HANDLE PLAYER DEATH"));
	//사망한 당일 아침으로 부활(아침으로 월드 롤백)
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
	
		//아침 세이브로 플레이어 로드
		RestartPlayer(DeadPlayerController);
		AfterRestartPlayer(DeadPlayerController, true);
	
		UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] LOAD MORNING COMPLETE"));
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] NO MORNING SAVE EXIST"));
	}

}


