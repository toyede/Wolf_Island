// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/GameModes/LobbyGameMode.h"

#include "Character/MainPlayerController.h"
#include "GameFramework/GameSession.h"
#include "Games/LobbyPlayerController.h"
#include "Games/PlayerSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Lobby/Lobby.h"

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	bUseSeamlessTravel = true;
	
	SetPlayerSlots();
	
	//기 입장 플레이어(서버) 슬롯 할당 - 서버는 PostLogin에서 감지가 안됨;;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (IsValid(PC))
		{
			PlayerControllers.Add(PC);
			AllocateSlot(PC);
			
			if (ALobbyPlayerController* LPC = Cast<ALobbyPlayerController>(PC))
			{
				if (LPC->LobbyWidget)
				{
					ULobby* Lobby = Cast<ULobby>(LPC->LobbyWidget);
					Lobby->RefreshInfo();
				}
			}
		}
	}
}

void ALobbyGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ALobbyGameMode::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	if (APlayerController* PC = Cast<APlayerController>(NewPlayer))
	{
		FString ID = PC->GetPlayerState<AMainPlayerState>()->GetPersistantId();
		UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] %s LOGIN"), *ID)
		PlayerControllers.Add(PC);
		AllocateSlot(PC);
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] NO PLAYER CONTROLLER ON POST LOGIN"))
	}
	
	Super::PostLogin(NewPlayer);
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	if (APlayerController* PC = Cast<APlayerController>(Exiting))
	{
		PlayerControllers.Remove(PC);
		RemoveSlot(PC);
	}
	
	Super::Logout(Exiting);
}

void ALobbyGameMode::SetPlayerSlots()
{
	TArray<AActor*> FoundSlots;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerSlot::StaticClass(), FoundSlots);
	
	for (AActor* FoundSlot : FoundSlots)
	{
		APlayerSlot* PlayerSlot = Cast<APlayerSlot>(FoundSlot);
		
		if (!PlayerSlot) continue;
		
		PlayerSlots.Add(PlayerSlot);
	}
	
	SlotsDone = true;
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] Player Slots Setting Done"))
}

void ALobbyGameMode::AllocateSlot(APlayerController* NewController)
{
	while (true)
	{
		if (SlotsDone) break;
	}
	
	for (APlayerSlot* PlayerSlot : PlayerSlots)
	{
		if (!IsValid(PlayerSlot)) continue;
		//빈슬롯에 할당
		if (!PlayerSlot->PlayerController)
		{			
			FString ID = NewController->GetPlayerState<AMainPlayerState>()->GetPersistantId();
			UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] %s Allocate Slot %s"), *ID, *PlayerSlot->Tags[0].ToString());
			if (AMainPlayerState* PS = NewController->GetPlayerState<AMainPlayerState>())
			{
				PS->SetRandomRole();
			} else
			{
				UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] No Player State on AllocateSlot"))
			}
			PlayerSlot->AddPlayer(NewController);
			return;
		}
	}
}

void ALobbyGameMode::RemoveSlot(APlayerController* RemovedController)
{
	for (APlayerSlot* PlayerSlot : PlayerSlots)
	{
		if (!IsValid(PlayerSlot)) continue;
		//해당 컨트롤러가 있는 슬롯을 삭제
		if (IsValid(PlayerSlot->PlayerController) && PlayerSlot->PlayerController == RemovedController)
		{
			FString ID = RemovedController->GetPlayerState<AMainPlayerState>()->GetPersistantId();
			UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE] %s Remove Player to Slot %s"), *ID, *PlayerSlot->Tags[0].ToString());
			PlayerSlot->RemovePlayer();
			return;
		}
	}
}

void ALobbyGameMode::RefreshSlots()
{
	for (APlayerSlot* PlayerSlot : PlayerSlots)
	{
		if (!IsValid(PlayerSlot)) continue;
		
		PlayerSlot->RefreshSlot();
	}
}

void ALobbyGameMode::RefreshSlot(APlayerController* TargetController)
{
	for (APlayerSlot* PlayerSlot : PlayerSlots)
	{
		if (!IsValid(PlayerSlot)) continue;
		
		if (IsValid(PlayerSlot->PlayerController) && PlayerSlot->PlayerController == TargetController)
		{
			PlayerSlot->RefreshSlot();
		}
	}
}

bool ALobbyGameMode::CheckAllPlayerReady()
{
	ALobbyPlayerController* LLPC = nullptr;
	bool IsAllReady = true;;
	
	for (APlayerController* PC : PlayerControllers)
	{
		if (ALobbyPlayerController* LPC = Cast<ALobbyPlayerController>(PC))
		{
			if (!LPC->IsReady) IsAllReady = false;
			
			if (LPC->IsLocalPlayerController()) LLPC = LPC;
		}
	}
	
	if (LLPC && LLPC->LobbyWidget)
	{
		if (ULobby* Lobby = Cast<ULobby>(LLPC->LobbyWidget))
		{
			Lobby->SwitchPlayButton(IsAllReady);
		}
	}
	
	return IsAllReady;
}

bool ALobbyGameMode::CheckRoleSelection()
{
	//선택된 역할 리스트
	TArray<ECharacterRole> SelectedRoles;
	
	//해당 로비의 플레이어를 돌며 선택한 역할 확인
	AMainGameState* GS = GetGameState<AMainGameState>();
	for (auto Player : GS->PlayerArray)
	{	
		//선택한 역할이 선택된 역할 리스트에 있으면 false 반환
		AMainPlayerState* PS = Cast<AMainPlayerState>(Player);
		if (SelectedRoles.Contains(PS->GetPlayerRole())) return false;
		//아니면 선택된 역할 리스트에 넣고 다음 플레이어 확인
		SelectedRoles.Add(PS->GetPlayerRole());
	}
	
	return true;
}

void ALobbyGameMode::RunGameTravel_Implementation()
{
	
}
