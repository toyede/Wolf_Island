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
#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

void AMainGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	MainGameInstance = Cast<UMainGameInstance>(GetGameInstance());
	
	//메인 메뉴에서 입장 시 슬롯에서 불러온 세이브 게임 데이터 로드
	if (MainGameInstance && MainGameInstance->CurrenSaveGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("Load Save Data From MainGameInstance"));
		CurrentSaveData = MainGameInstance->CurrenSaveGame;
		PlayersSaveData = CurrentSaveData->Players;
	}
	//에디터에서 월드로 바로 입장 시 테스트 세이브 게임 데이터 생성 및 로드
	else
	{
		//테스트 세이브 게임 데이터가 있으면 불러오기
		if (UGameplayStatics::DoesSaveGameExist(TEXT("TEST001_")+MapName,0))
		{
			UE_LOG(LogTemp, Warning, TEXT("Load TEST SAVE GAME"));
			CurrentSaveData = Cast<UMainSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("TEST001_")+MapName, 0));
			PlayersSaveData = CurrentSaveData->Players;
		}
		//테스트 세이브 게임 데이터가 없으면 생성
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Create TEST SAVE GAME"));
			CurrentSaveData = Cast<UMainSaveGame>(UGameplayStatics::CreateSaveGameObject(UMainSaveGame::StaticClass()));
			CurrentSaveData->SlotName = TEXT("TEST001_")+MapName;
			CurrentSaveData->WorldName = MapName;
			UGameplayStatics::SaveGameToSlot(CurrentSaveData, CurrentSaveData->SlotName, 0);
		}
	}
}

void AMainGameMode::StartPlay()
{
	Super::StartPlay();
	
	LoadWorld();
	
	//자동 저장 타이머
	GetWorld()->GetTimerManager().SetTimer(
		AutoSaveTimer,
		this,
		&AMainGameMode::SaveWorld,
		AutoSaveInterval * 60.0f,
		true);
}

void AMainGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	//채팅 테스트하는 데 플레이어 이름이 너무 길어서 귀염뽀짝 짧은 이름으로 재설정 해주는 개발용 코드
	if (!NewPlayer) return;

	AMainPlayerState* PS = Cast<AMainPlayerState>(NewPlayer->PlayerState);
	if (!PS) return;

	static int32 Counter = 1;
	static const TArray<FString> Adjs = {
		TEXT("귀여운"), TEXT("빠른"), TEXT("용감한"), TEXT("조용한"),
		TEXT("무거운"), TEXT("느긋한"), TEXT("멍청한"), TEXT("조그만"),
		TEXT("지루한"), TEXT("무서운"), TEXT("재밌는"), TEXT("거대한"),
		TEXT("발정난"), TEXT("옹골진"), TEXT("섹시한"), TEXT("길쭉한"),
	};
	static const TArray<FString> Nouns = {
		TEXT("여우"), TEXT("늑대"), TEXT("토끼"), TEXT("곰"),
		TEXT("고라니"), TEXT("멧돼지"), TEXT("개"), TEXT("고양이"),
		TEXT("닭"), TEXT("땃쥐"), TEXT("까마귀"), TEXT("사슴"),
		TEXT("코끼리"), TEXT("다람쥐"), TEXT("매"), TEXT("살쾡이")
	};

	int32 A = FMath::RandRange(0, Adjs.Num()-1);
	int32 N = FMath::RandRange(0, Nouns.Num()-1);
	
	FString NewID = Adjs[A]+" "+Nouns[N]+FString::FromInt(Counter++);
	//FString NewID = "TESTER"+FString::FromInt(Counter++);
	
	PS->SetPlayerName(NewID);
	
	PS->SetPlayerTag(NewID);
	
	AMainGameState* GS = GetGameState<AMainGameState>();
	FChattingData Chat = FChattingData(
		TEXT("알림"),PS->GetPlayerName()+TEXT(" 님이 접속했습니다."), EMessageType::NOTICE);
	
	GS->AddChattingMessage(Chat);
}

void AMainGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	
	GetWorld()->GetTimerManager().SetTimerForNextTick([this, NewPlayer]
	{
		StartingNewPlayer(NewPlayer);
	});
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

void AMainGameMode::StartingNewPlayer(APlayerController* NewPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("[SINGLEPLAY] Starting New Player"));
	//로그인 시 해당 플레이어의 저장된 정보를 불러온다.
	AMainPlayerState* PS = Cast<AMainPlayerState>(NewPlayer->PlayerState);
	
	if (PS)
	{
		LoadPlayer(PS);
	}
}

void AMainGameMode::SetActorCache()
{
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
		UE_LOG(LogTemp, Warning, TEXT("SAVE FAILED"));
		return;
	}
	
	//저장 가능 액터 데이터 저장
	//저장 인터페이스를 가진 모든 액터 가져오기
	TArray<AActor*> SaveActors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), USaveInterface::StaticClass(), SaveActors);
	
	Save->SavedActors.Empty();
	
	//각 액터의 저장 코드 실행
	for (AActor* Actor : SaveActors)
	{
		if (ISaveInterface* Savable = Cast<ISaveInterface>(Actor))
		{
			FActorSaveData Data;
			Savable->Execute_SaveData(Actor, Data);
			UE_LOG(LogTemp, Warning, TEXT("[%s] Save Actor [%s]"), *Actor->GetName(), *Data.ActorID.ToString())
			//세이브 파일에 액터 저장 데이터 추가
			Save->SavedActors.FindOrAdd(Data.ActorID) = Data;
		}
	}	
	
	//폴리지 데이터 저장
	Save->RemovedFoliages.Append(RemovedFoliageData);
	RemovedFoliageData.Empty();
	
	//플레이어 데이터 저장
	AMainGameState* GS = GetGameState<AMainGameState>();
	//월드에 있는 플레이어 순회
	for (APlayerState* PS : GS->PlayerArray)
	{
		//각 플레이어의 저장 코드 실행
		AMainPlayerState* MPS = Cast<AMainPlayerState>(PS);
		SavePlayer(MPS);
	}
	//세이브 파일에 플레이어 저장 데이터 추가
	Save->Players = PlayersSaveData;
	
	//세이브 파일 슬롯에 저장
	UGameplayStatics::SaveGameToSlot(CurrentSaveData, CurrentSaveData->SlotName, 0);
	UE_LOG(LogTemp, Warning, TEXT("Test Save at %s"), *CurrentSaveData->SlotName);
	
	
	FChattingData Chat = FChattingData(
		TEXT("SYSTEM"),TEXT("자동 저장 완료"), EMessageType::NOTICE);
	GS->AddChattingMessage(Chat);
}

void AMainGameMode::LoadWorld()
{
	if (!HasAuthority()) return;
	UE_LOG(LogTemp, Warning, TEXT("Test Load at %s"), *CurrentSaveData->SlotName);
	
	UMainSaveGame* Save =
		Cast<UMainSaveGame>(UGameplayStatics::LoadGameFromSlot(CurrentSaveData->SlotName, 0));

	if (!Save) return;
	
	//저장된 액터가 있으면 액터 로드
	if (Save->SavedActors.Num() != 0)
	{
		//초기 상태 액터 캐시 생성
		SetActorCache();
	
		//삭제된 기존 액터는 삭제
		for (auto& Pair : ActorCache)
		{
			if (!Save->SavedActors.Contains(Pair.Key))
			{
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
	
	UE_LOG(LogTemp, Warning, TEXT("Load at %s COMPLETE"), *CurrentSaveData->SlotName);
}

void AMainGameMode::SavePlayer(AMainPlayerState* PlayerState)
{
	//해당 플레이어의 아이디를 키로하는 맵에 데이터를 저장.
	//FString PlayerID = FString::FromInt(TargetPlayer->GetController()->PlayerState->GetPlayerId());
	//FString PlayerID = TargetPlayer->GetController()->PlayerState->GetUniqueId()->ToString();
	FString PlayerID = TEXT("TESTER");
	FPlayerSaveData& PlayerSaveData = PlayersSaveData.FindOrAdd(PlayerID);
	
	PlayerSaveData.PlayerID = PlayerID;
	
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
		FMemoryWriter InventoryWriter(PlayerSaveData.InventoryBinaryData, true);
		FObjectAndNameAsStringProxyArchive InventoryArchive(InventoryWriter, true);
		InventoryArchive.ArIsSaveGame = true;
		TargetPlayer->InventoryComponent->Serialize(InventoryArchive);
		
		FMemoryWriter StatusWriter(PlayerSaveData.StatusBinaryData, true);
		FObjectAndNameAsStringProxyArchive StatusArchive(StatusWriter, true);
		StatusArchive.ArIsSaveGame = false;
		TargetPlayer->StatusComponent->Serialize(StatusArchive);
	}
	
	//아이템 데이터는 플레이어 스테이트에 있는 것을 저장.
	//<?>인벤토리 업데이트 할때마다 플레이어 스테이트에 아이템 데이터가 저장됨.
	PlayerSaveData.InventoryItems = PlayerState->GetItems();
	
	UE_LOG(LogTemp, Warning, TEXT("Save Player ID: %s | PlayersSaveData Num : %d"), *PlayerID, PlayersSaveData.Num());
}

bool AMainGameMode::LoadPlayer(AMainPlayerState* PlayerState)
{
	ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerState->GetPawn());
	
	//해당 플레이어 스테이트를 가진 컨트롤러의 아이디로 조회.
	//FString PlayerID = FString::FromInt(TargetPlayer->GetController()->PlayerState->GetPlayerId());
	//FString PlayerID = PlayerCharacter->GetController()->PlayerState->GetUniqueId().ToString();
	FString PlayerID = TEXT("TESTER");
	
	//저장된 플레이어 목록 중 해당 플레이어가 없으면 로드 False.
	if (PlayersSaveData.Find(PlayerID) == NULL) return false;
	
	FPlayerSaveData& PlayerSaveData = PlayersSaveData[PlayerID];
	
	PlayerState->SetItemsData(PlayerSaveData.InventoryItems);
	
	
	if (PlayerCharacter)
	{
		PlayerCharacter->SetActorTransform(PlayerSaveData.Transform);
		PlayerCharacter->GetCharacterMovement()->Velocity = PlayerSaveData.Velocity;
		PlayerCharacter->GetController()->SetControlRotation(PlayerSaveData.ControlRotation);
	}
	
	if (AMainPlayer* TargetPlayer = Cast<AMainPlayer>(PlayerCharacter))
	{
		FMemoryReader InventoryReader(PlayerSaveData.InventoryBinaryData, true);
		FObjectAndNameAsStringProxyArchive InventoryArchive(InventoryReader, true);
		InventoryArchive.ArIsSaveGame = true;
		TargetPlayer->InventoryComponent->Serialize(InventoryArchive);
		TargetPlayer->InventoryComponent->InventoryChanged();
	
		FMemoryReader StatusReader(PlayerSaveData.StatusBinaryData, true);
		FObjectAndNameAsStringProxyArchive StatusArchive(StatusReader, true);
		StatusArchive.ArIsSaveGame = false;
		TargetPlayer->StatusComponent->Serialize(StatusArchive);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Load Player ID: %s"), *PlayerID);
	return true;
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
	//월드를 저장하고, 그 저장된 걸 복제해서 아침 세이브데이터로 저장.
	//CurrentSaveData 는 그 뒤로 자동 저장되어 계속 덮어씌워지고, MorningSaveData는 아침 때로 유지.
	//라고 생각했었는데 생각해보니까 이러면 월드 종료하면 아침 데이터가 날아감...
	//저장 슬롯으로 접미사에 _morning을 갖는 슬롯을 만들어 저장해놓는 게 좋겠다.
	SaveWorld();
	UMainSaveGame* NewSave = DuplicateSaveData(CurrentSaveData);
	MorningSaveData = NewSave;
	MorningSaveData->SlotName += TEXT("_morning");
	
	//세이브 파일 슬롯에 저장(슬롯 이름에 _morning 붙여서)
	UGameplayStatics::SaveGameToSlot(MorningSaveData, MorningSaveData->SlotName, 0);
	
	AMainGameState* GS = GetGameState<AMainGameState>();
	FChattingData Chat = FChattingData(
		TEXT("SYSTEM"),TEXT("아침 데이터 저장"), EMessageType::NOTICE);
	GS->AddChattingMessage(Chat);
}

//싱글에서 죽었을 때
void AMainGameMode::HandlePlayerDeath(AController* DeadPlayerController)
{
	//사망한 당일 아침으로 부활(아침으로 월드 롤백)
	//아침 데이터 슬롯 구하기
	FString MorningSlotName = CurrentSaveData->SlotName+TEXT("_morning");
	//아침 데이터 슬롯에서 데이터 가져오기
	UMainSaveGame* Save =
		Cast<UMainSaveGame>(UGameplayStatics::LoadGameFromSlot(MorningSlotName, 0));
	//현재 세이브 파일을 아침 데이터로 교체
	CurrentSaveData = Save;
	//그 세이브 파일을 기반으로 월드 로드
	LoadWorld();
}

bool AMainGameMode::RespawnPlayer(AController* TargetPlayerController)
{
	//플레이어 리스폰 시 역할에 따른 캐릭터 소환 후 데이터 동기화
	AMainPlayerState* PlayerState = Cast<AMainPlayerState>(TargetPlayerController->PlayerState);
	//FString PlayerID = PlayerState->GetUniqueId().ToString();
	FString PlayerID = TEXT("TESTER");
	
	if (!PlayerState) return false;
	
	int32 Index = 0;
	
	switch (PlayerState->GetPlayerRole())
	{
	case ECharacterRole::CAPTAIN:
		{
			Index = 1;
			break;
		}
	case ECharacterRole::CHEF:
		{	
			Index = 2;
			break;
		}
	case ECharacterRole::MECHANIC:
		{
			Index = 3;
			break;
		}
	case ECharacterRole::SOLDIER:
		{
			Index = 4;
			break;
		}
	case ECharacterRole::NONE:
		{
			Index = 0;
			break;
		}
	}
	
	//해당 역할의 캐릭터 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	int32 I = FMath::RandRange(0, SpawnPoints.Num()-1);
	FVector SpawnLocation = SpawnPoints[I];
	FRotator SpawnRotation = FRotator(FRotator::ZeroRotator);
	FTransform SpawnTransform = FTransform(SpawnRotation, SpawnLocation);
	
	AMainPlayer* SpawnedPlayer = 
		GetWorld()->SpawnActorDeferred<AMainPlayer>(
		PlayerRoleClassList[Index], SpawnTransform,
		nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	SpawnedPlayer->FinishSpawning(SpawnTransform);
	UE_LOG(LogTemp, Warning, TEXT("Spawned Complete for new user"))
	
	TargetPlayerController->Possess(SpawnedPlayer);
	
	LoadPlayer(PlayerState);
	
	return true;
}


