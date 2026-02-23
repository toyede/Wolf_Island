// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/GameModes/MultiGameMode.h"

#include "Character/MainPlayerController.h"
#include "Games/MainPlayerState.h"
#include "Widgets/RoleSelection/RoleButton.h"

void AMultiGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
}

void AMultiGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

void AMultiGameMode::StartingNewPlayer(APlayerController* NewPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("[%hs][MULTIPLAY] Starting New Player"), NewPlayer->IsLocalController()?"SERVER":"CLIENT");
	
	AMainPlayerController* NewMainPlayerController = Cast<AMainPlayerController>(NewPlayer);
	//플레이어 리스폰 시 역할에 따른 캐릭터 소환 후 데이터 동기화
	AMainPlayerState* PlayerState = Cast<AMainPlayerState>(NewPlayer->PlayerState);
	if (!PlayerState) return;
	
	//FString PlayerID = PlayerState->GetUniqueId().ToString();
	FString PlayerID = TEXT("TESTER");
	
	UE_LOG(LogTemp, Warning, TEXT("PlayersSaveData num : %d"), PlayersSaveData.Num());
	
	//기존 입장 플레이어 라면?
	if (PlayersSaveData.Contains(PlayerID))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%hs][%s] is old user"), NewPlayer->IsLocalController()?"SERVER":"CLIENT", *PlayerID);
		FPlayerSaveData& PlayerSaveData = PlayersSaveData[PlayerID];
		
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
		
		AMainPlayer* SpawnedPlayer = 
			GetWorld()->SpawnActorDeferred<AMainPlayer>(
				PlayerRoleClassList[Index], PlayerSaveData.Transform,
				nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		SpawnedPlayer->FinishSpawning(PlayerSaveData.Transform);
		UE_LOG(LogTemp, Warning, TEXT("Spawned Complete for old user"))
		
		NewPlayer->Possess(SpawnedPlayer);
		
		//플레이어 데이터 로드
		LoadPlayer(PlayerState);
	}
	//새로 입장한 플레이어면
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%hs][%s] is new user"), NewPlayer->IsLocalController()?"SERVER":"CLIENT", *PlayerID);
		
		//역할 고르기 UI 출력
		GetWorld()->GetTimerManager().SetTimerForNextTick([this, NewMainPlayerController]()
		{
			NewMainPlayerController->Client_OpenSelectionUI();
		});
	}
}

void AMultiGameMode::AllocatePlayer(APlayerController* NewPlayer)
{
	AMainPlayerController* NewMainPlayerController = Cast<AMainPlayerController>(NewPlayer);
	AMainPlayerState* PS = Cast<AMainPlayerState>(NewPlayer->PlayerState);
	
	int32 Index = 0;
	
	switch (PS->GetPlayerRole())
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
	
	UE_LOG(LogTemp, Warning, TEXT("[ROLE: %d]Spawning Player %s [New]"), PS->GetPlayerRole(), *NewPlayer->GetName());
	AMainPlayer* SpawnedPlayer = 
		GetWorld()->SpawnActorDeferred<AMainPlayer>(
		PlayerRoleClassList[Index], SpawnTransform,
			nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	SpawnedPlayer->FinishSpawning(SpawnTransform);
	
	NewPlayer->Possess(SpawnedPlayer);
	NewMainPlayerController->Client_SetInputModeGame();
}


void AMultiGameMode::HandlePlayerDeath(AController* DeadPlayerController)
{
	RespawnPlayer(DeadPlayerController);	
}
