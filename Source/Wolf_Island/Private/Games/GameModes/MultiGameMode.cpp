// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/GameModes/MultiGameMode.h"

#include "Character/MainPlayerController.h"
#include "Games/MainPlayerState.h"
#include "Widgets/RoleSelection/RoleButton.h"

AMultiGameMode::AMultiGameMode()
{
	bPauseable = false;
}

void AMultiGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
}

void AMultiGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	
	AMainPlayerController* NewMainPlayerController = Cast<AMainPlayerController>(NewPlayer);
	AMainPlayerState* PlayerState = Cast<AMainPlayerState>(NewMainPlayerController->PlayerState);
	if (PlayerState->GetPlayerRole() == ECharacterRole::NONE)
	{
		NewMainPlayerController->Client_OpenSelectionUI();
	}
}

void AMultiGameMode::RestartPlayer(AController* NewPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("Call RestartPlayer on MultiGamemode"));

	Super::RestartPlayer(NewPlayer);
}

bool AMultiGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	AMainPlayerState* PlayerState = Cast<AMainPlayerState>(Player->PlayerState);
	UE_LOG(LogTemp, Warning, TEXT("Check Role om ShouldSpawn : %d"), PlayerState->GetPlayerRole());
	if (!PlayerState) return false;
	
	if (PlayerState->GetPlayerRole() == ECharacterRole::NONE) return false;
	
	UE_LOG(LogTemp, Warning, TEXT("Allow to Spawn"));
	return Super::ShouldSpawnAtStartSpot(Player);
}

//RestartPlayer 실행 시 호출되는 함수... 플레이어 폰 생성
APawn* AMultiGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	AMainPlayerState* PS = Cast<AMainPlayerState>(NewPlayer->PlayerState);

	int32 RoleIndex = static_cast<int32>(PS->GetPlayerRole());

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	int32 PointIndex = FMath::RandRange(0, SpawnPoints.Num()-1);
	FVector SpawnLocation = SpawnPoints[PointIndex];
	FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

	AMainPlayer* Player = GetWorld()->SpawnActor<AMainPlayer>(
		PlayerRoleClassList[RoleIndex],
		SpawnTransform,
		Params);
	
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawned Player Actor is NULL"))
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawned Player Actor is NOT NULL"))
	}

	return Player;
}

APawn* AMultiGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer,
	const FTransform& SpawnTransform)
{
	AMainPlayerState* PS = Cast<AMainPlayerState>(NewPlayer->PlayerState);

	int32 RoleIndex = static_cast<int32>(PS->GetPlayerRole());

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	int32 PointIndex = FMath::RandRange(0, SpawnPoints.Num()-1);
	FVector SpawnLocation = SpawnPoints[PointIndex];
	FTransform SpawnTransforms(FRotator::ZeroRotator, SpawnLocation);

	AMainPlayer* Player = GetWorld()->SpawnActor<AMainPlayer>(
		PlayerRoleClassList[RoleIndex],
		SpawnTransforms,
		Params);
	
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawned Player Actor is NULL"))
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawned Player Actor is NOT NULL"))
	}

	return Player;
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
	
	UE_LOG(LogTemp, Warning, TEXT("[Role] : %d"), PlayerState->GetPlayerRole());
	
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
		
		GetWorld()->GetTimerManager().SetTimerForNextTick([this, PlayerState]
		{
			//플레이어 데이터 로드
			LoadPlayer(PlayerState);
		});
	}
	//새로 입장한 플레이어면
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%hs][%s] is new user"), NewPlayer->IsLocalController()?"SERVER":"CLIENT", *PlayerID);
		
		//역할 고르기 UI 출력 > UI에서 역할 고른 후 AllocatePlayer 호출
		GetWorld()->GetTimerManager().SetTimerForNextTick([this, NewMainPlayerController]()
		{
			NewMainPlayerController->Client_OpenSelectionUI();
		});
	}
}

void AMultiGameMode::HandlePlayerDeath(AController* DeadPlayerController)
{
	RestartPlayer(DeadPlayerController);
}
