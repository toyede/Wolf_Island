// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/GameModes/MainGameMode.h"

#include "Character/MainPlayer.h"
#include "Components/InventoryComponent.h"
#include "Components/StatusComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "Games/MainGameState.h"
#include "Games/MainPlayerState.h"
#include "Games/MainSaveGame.h"
#include "Games/SaveInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

class AMainGameState;

void AMainGameMode::StartPlay()
{
	Super::StartPlay();
	
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
	
	//FString NewID = Adjs[A]+" "+Nouns[N]+FString::FromInt(Counter++);
	FString NewID = "TESTER"+FString::FromInt(Counter++);
	
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
	
	//로그인 시 해당 플레이어의 저장된 정보를 불러온다.
	AMainPlayer* Player = Cast<AMainPlayer>(NewPlayer->GetPawn());
	if (Player)
	{
		LoadPlayer(Player);
	}
}

void AMainGameMode::Logout(AController* Exiting)
{
	//로그아웃 시 플레이어 정보 저장
	AMainPlayer* Player = Cast<AMainPlayer>(Exiting->GetPawn());
	if (Player)
	{
		SavePlayer(Player);
	}
	
	Super::Logout(Exiting);
}

void AMainGameMode::Save()
{
	UE_LOG(LogTemp, Warning, TEXT("Test Save on %hs"), HasAuthority()?"SERVER":"CLIENT");
	UMainSaveGame* Save =
		Cast<UMainSaveGame>(UGameplayStatics::CreateSaveGameObject(UMainSaveGame::StaticClass()));
	
	TArray<AActor*> SaveActors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), USaveInterface::StaticClass(), SaveActors);
	
	for (AActor* Actor : SaveActors)
	{
		if (ISaveInterface* Savable = Cast<ISaveInterface>(Actor))
		{
			FActorSaveData Data;
			Savable->SaveData(Data);
			Save->SavedActors.Add(Data);
		}
	}	
	
	AMainGameState* GS = GetGameState<AMainGameState>();
	for (APlayerState* PS : GS->PlayerArray)
	{
		AMainPlayer* Player = Cast<AMainPlayer>(PS->GetPawn());
		SavePlayer(Player);
	}
	
	Save->Players = PlayersSaveData;
	
	UGameplayStatics::SaveGameToSlot(Save, TEXT("TestSlot"), 0);
}

void AMainGameMode::Load()
{
	if (!HasAuthority()) return;
	UE_LOG(LogTemp, Warning, TEXT("Test Load on %hs"), HasAuthority()?"SERVER":"CLIENT");
	UMainSaveGame* Save =
		Cast<UMainSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("TestSlot"), 0));

	if (!Save) return;

	for (FActorSaveData& Data : Save->SavedActors)
	{
		AActor* NewActor = GetWorld()->SpawnActor<AActor>(
		Data.ActorClass,
		Data.Transform);

		if (ISaveInterface* Savable = Cast<ISaveInterface>(NewActor))
		{
			Savable->LoadData(Data);
		}
	}
	
	PlayersSaveData = Save->Players;
	
	AMainGameState* GS = GetGameState<AMainGameState>();
	for (APlayerState* PS : GS->PlayerArray)
	{
		AMainPlayer* Player = Cast<AMainPlayer>(PS->GetPawn());
		LoadPlayer(Player);
	}
}

void AMainGameMode::SavePlayer(AMainPlayer* TargetPlayer)
{
	FString PlayerID = FString::FromInt(TargetPlayer->GetController()->PlayerState->GetPlayerId());
	
	if (PlayersSaveData.Find(PlayerID) == NULL)
	{
		PlayersSaveData.Add(PlayerID);
	}
	
	FPlayerSaveData& PlayerSaveData = PlayersSaveData[PlayerID];
	
	PlayerSaveData.PlayerID = PlayerID;
	PlayerSaveData.Transform = TargetPlayer->GetActorTransform();
	PlayerSaveData.Velocity = TargetPlayer->GetVelocity();
	PlayerSaveData.ControlRotation = TargetPlayer->GetControlRotation();
	
	PlayerSaveData.InventoryItems = TargetPlayer->InventoryComponent->GetInventory();
	
	FMemoryWriter InventoryWriter(PlayerSaveData.InventoryBinaryData, true);
	FObjectAndNameAsStringProxyArchive InventoryArchive(InventoryWriter, true);
	InventoryArchive.ArIsSaveGame = true;
	TargetPlayer->InventoryComponent->Serialize(InventoryArchive);
	
	FMemoryWriter StatusWriter(PlayerSaveData.StatusBinaryData, true);
	FObjectAndNameAsStringProxyArchive StatusArchive(StatusWriter, true);
	StatusArchive.ArIsSaveGame = false;
	TargetPlayer->StatusComponent->Serialize(StatusArchive);
	
	UE_LOG(LogTemp, Warning, TEXT("Save Player ID: %s"), *PlayerID);
}

bool AMainGameMode::LoadPlayer(AMainPlayer* TargetPlayer)
{
	FString PlayerID = FString::FromInt(TargetPlayer->GetController()->PlayerState->GetPlayerId());
	
	if (PlayersSaveData.Find(PlayerID) == NULL) return false;
	
	FPlayerSaveData& PlayerSaveData = PlayersSaveData[PlayerID];
	
	FMemoryReader InventoryReader(PlayerSaveData.InventoryBinaryData, true);
	FObjectAndNameAsStringProxyArchive InventoryArchive(InventoryReader, true);
	InventoryArchive.ArIsSaveGame = true;
	TargetPlayer->InventoryComponent->Serialize(InventoryArchive);
	TargetPlayer->InventoryComponent->InventoryChanged();
	
	FMemoryReader StatusReader(PlayerSaveData.StatusBinaryData, true);
	FObjectAndNameAsStringProxyArchive StatusArchive(StatusReader, true);
	StatusArchive.ArIsSaveGame = false;
	TargetPlayer->StatusComponent->Serialize(StatusArchive);
	
	//TargetPlayer->InventoryComponent->SetInventoryContents(PlayerSaveData.InventoryItems);
	
	TargetPlayer->SetActorTransform(PlayerSaveData.Transform);
	TargetPlayer->GetCharacterMovement()->Velocity = PlayerSaveData.Velocity;
	TargetPlayer->GetController()->SetControlRotation(PlayerSaveData.ControlRotation);
	
	UE_LOG(LogTemp, Warning, TEXT("Load Player ID: %s"), *PlayerID);
	return true;
}


