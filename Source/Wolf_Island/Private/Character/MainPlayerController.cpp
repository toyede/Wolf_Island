// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MainPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "Games/MainGameInstance.h"
#include "Games/MainHUD.h"
#include "Widgets/Chatting/ChattingPanel.h"
#include "Games/MainGameState.h"
#include "Games/GameModes/MultiGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/FishTrap/FishTrapScreen.h"
#include "Widgets/BaseButton.h"
#include "Widgets/PlayerHUD.h"
#include "Widgets/MainMenu/PauseMenu.h"
#include "Widgets/RoleSelection/RoleSelection.h"
#include "Widgets/DeathScreen.h"
#include "Moon/MoonlightInfectionSystem.h"
#include "Actors/SpectatorCameraActor.h"
#include "Components/Button.h"
#include "Widgets/Setting/SettingsWidget.h"
#include "Components/InventoryComponent.h"

//TODO: 일단 바인딩 해제 코드 안넣어봄.
void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	HUD = Cast<AMainHUD>(GetHUD());
	
	if (ChattingPanelClass && IsLocalController())
	{
		ChattingPanel = CreateWidget<UChattingPanel>(this, ChattingPanelClass);
		if (ChattingPanel)
		{
			ChattingPanel->AddToViewport();
		}
	}
	
	if (PauseWidgetClass && IsLocalController() && !PauseMenu)
	{
		PauseMenu = CreateWidget<UPauseMenu>(this, PauseWidgetClass);
		
		if (PauseMenu)
		{
			if (PauseMenu->ResumeButton) PauseMenu->ResumeButton->OnClicked.AddUniqueDynamic(this, &AMainPlayerController::OnResume);
			if (PauseMenu->SettingButton) PauseMenu->SettingButton->OnClicked.AddUniqueDynamic(this, &AMainPlayerController::OnSetting);
			if (PauseMenu->QuitButton) PauseMenu->QuitButton->OnClicked.AddUniqueDynamic(this, &AMainPlayerController::OnQuit);
			
			PauseMenu->AddToViewport(10);
			HidePauseMenu();
		}
	}
	
	MainGameInstance = Cast<UMainGameInstance>(GetGameInstance());
	MainGameMode = Cast<AMainGameMode>(GetWorld()->GetAuthGameMode());
	MainPlayerState = GetPlayerState<AMainPlayerState>();
	MainGameState = Cast<AMainGameState>(GetWorld()->GetGameState());
	
	// Selection UI는 OnRep_PlayerState에서만 처리 (중복 호출 방지)
	// 서버 로컬 플레이어는 OnRep_PlayerState가 호출되지 않으므로 여기서 처리
	if (IsLocalController() && HasAuthority() && MainPlayerState && MainGameState && MainGameState->IsMulti)
	{
		if (MainPlayerState->GetPlayerRole() == ECharacterRole::NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("Open Selection UI in Player Controller (Server Local)"))
			Client_OpenSelectionUI();
		} else
		{
			UE_LOG(LogTemp, Warning, TEXT("[PC] Can't open Selection UI. Player role is %d"), MainPlayerState->GetPlayerRole())
		}
	}
}

void AMainPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 월드 전환이나 객체 파괴 시 생성된 위젯들 정리
	if (ChattingPanel)
	{
		ChattingPanel->RemoveFromParent();
		ChattingPanel = nullptr;
	}

	if (PauseMenu)
	{
		PauseMenu->RemoveFromParent();
		PauseMenu = nullptr;
	}

	if (SettingsWidget)
	{
		SettingsWidget->RemoveFromParent();
		SettingsWidget = nullptr;
	}

	if (RoleSelectionWidget)
	{
		RoleSelectionWidget->RemoveFromParent();
		RoleSelectionWidget = nullptr;
	}

	if (DeathScreenWidget)
	{
		DeathScreenWidget->RemoveFromParent();
		DeathScreenWidget = nullptr;
	}

	if (PlayerHUD)
	{
		PlayerHUD->RemoveFromParent();
		PlayerHUD = nullptr;
	}

	GetWorldTimerManager().ClearAllTimersForObject(this);

	Super::EndPlay(EndPlayReason);
}

void AMainPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// 향상된 입력 컴포넌트
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(ChatAction, ETriggerEvent::Started, this, &AMainPlayerController::ToggleChatMode);
		EnhancedInputComponent->BindAction(ESCAction, ETriggerEvent::Started, this, &AMainPlayerController::TogglePause);
	
		// 관전 가능한 상태
		if (SpectateNextAction)
		{
			EnhancedInputComponent->BindAction(SpectateNextAction, ETriggerEvent::Started, this, &AMainPlayerController::SwitchSpectateTarget);
		}
	}
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InputMappingContext, 1);
	}
}

void AMainPlayerController::InitPlayerState()
{
	Super::InitPlayerState();
}

void AMainPlayerController::OnPossess(APawn* InPawn)
{
	UE_LOG(LogTemp, Warning, TEXT("[PLAYER CONTROLLER] ON POSSESSED"));
	Super::OnPossess(InPawn);
}

void AMainPlayerController::OnUnPossess()
{
	UE_LOG(LogTemp, Warning, TEXT("[PLAYER CONTROLLER] UNPOSSESSED"));
	GetWorldTimerManager().ClearAllTimersForObject(this);
	
	if (IsLocalController() && PlayerHUD)
	{
		PlayerHUD->RemoveFromParent();
		PlayerHUD = nullptr;
	}
	
	Super::OnUnPossess();
}

void AMainPlayerController::SetPlayerHUD(AMainPlayer* OwnerPlayer)
{
	if (!OwnerPlayer || !IsLocalController()) return;

	if (!HUDClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[PC] HUDClass is NOT assigned in Blueprint!"));
		return;
	}

	if (!PlayerHUD)
	{
		PlayerHUD = CreateWidget<UPlayerHUD>(this, HUDClass);
		if (PlayerHUD)
		{
			PlayerHUD->AddToViewport();
		}
	}

	if (PlayerHUD)
	{
		PlayerHUD->SetPlayerRef(OwnerPlayer);
		OwnerPlayer->SetHUDWidget(PlayerHUD);
	}
}

void AMainPlayerController::ToggleChatMode()
{
	if (IsChat) ExitChatMode();
	else EnterChatMode();
}

void AMainPlayerController::TogglePause()
{
	if (IsPause) HidePauseMenu();
	else DisplayPauseMenu();
}

void AMainPlayerController::EnterChatMode()
{
	IsChat = true;
	
	FInputModeGameAndUI Mode;
	SetInputMode(Mode);
	SetShowMouseCursor(true);

	if (ChattingPanel && ChattingPanel->ChattingInputBox)
	{
		Mode.SetWidgetToFocus(ChattingPanel->ChattingInputBox->TakeWidget());
	}

	if (ChattingPanel)
	{
		ChattingPanel->FocusInput();
	}
}

void AMainPlayerController::ExitChatMode()
{
	IsChat = false;
	
	FInputModeGameOnly Mode;
	SetInputMode(Mode);
	SetShowMouseCursor(false);

	if (ChattingPanel)
	{
		ChattingPanel->ClearFocusInput();
	}
}

void AMainPlayerController::DisplayPauseMenu()
{
	if (!PauseMenu) return;

	IsPause = true;
	bool GamePause = !Cast<AMultiGameMode>(GetWorld()->GetAuthGameMode());
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE MULTI?] : %d"), GamePause)
	bShowMouseCursor = true;

	FInputModeUIOnly Mode;
	SetInputMode(Mode);

	UGameplayStatics::SetGamePaused(GetWorld(), GamePause);

	PauseMenu->SetVisibility(ESlateVisibility::Visible);
	PauseMenu->SetKeyboardFocus();
}

void AMainPlayerController::HidePauseMenu()
{
	if (!PauseMenu) return;

	IsPause = false;
	bShowMouseCursor = false;

	FInputModeGameOnly Mode;
	SetInputMode(Mode);

	UGameplayStatics::SetGamePaused(GetWorld(), false);

	PauseMenu->SetVisibility(ESlateVisibility::Collapsed);
}


void AMainPlayerController::Client_OpenFishTrapUI_Implementation(class AFishTrap* TargetTrap, class AActor* Interactor)
{
	if (FishTrapScreenClass && TargetTrap)
	{
		UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(this, FishTrapScreenClass);
		
		if (UFishTrapScreen* FishTrapScreen = Cast<UFishTrapScreen>(CreatedWidget))
		{
			FishTrapScreen->InitializeScreen(TargetTrap, Interactor);
			FishTrapScreen->SetIsFocusable(true);
			FishTrapScreen->AddToViewport();

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(FishTrapScreen->TakeWidget());
			
			SetInputMode(InputMode);
			SetShowMouseCursor(true);
		}
	}
}

void AMainPlayerController::Request_SendChat(FChattingData NewChattingData)
{
	if (HasAuthority())
	{
		SendChat(NewChattingData);
	} else
	{
		Server_SendChat(NewChattingData);
	}
}

void AMainPlayerController::Server_SendChat_Implementation(FChattingData NewChattingData)
{
	SendChat(NewChattingData);
}

void AMainPlayerController::SendChat(FChattingData NewChattingData)
{
	if (!MainGameState) return;
	
	MainGameState->AddChattingMessage(NewChattingData);
}

void AMainPlayerController::Client_EndSelection_Implementation()
{
	FInputModeGameOnly Mode;
	SetInputMode(Mode);
	SetShowMouseCursor(false);
	if (RoleSelectionWidget)
	{
		RoleSelectionWidget->RemoveFromParent();
		RoleSelectionWidget = nullptr;
	}
}

void AMainPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	MainPlayerState = GetPlayerState<AMainPlayerState>();
	
	if (!MainGameState)
	{
		MainGameState = Cast<AMainGameState>(GetWorld()->GetGameState());
	}
   
	// 2. 널 체크 추가: MainGameState가 유효할 때만 IsMulti 변수에 접근
	if (MainPlayerState && MainGameState && MainGameState->IsMulti)
	{
		if (MainPlayerState->GetPlayerRole() == ECharacterRole::NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("[PC|%s] Open Selection UI in Player Controller"), *MainPlayerState->GetPersistantId())
			Client_OpenSelectionUI();
		} else
		{
			UE_LOG(LogTemp, Warning, TEXT("[PC|%s] Can't open Selection UI. Player role is %d"), *MainPlayerState->GetPersistantId(), MainPlayerState->GetPlayerRole())
		}
	} else
	{
		//UE_LOG(LogTemp, Warning, TEXT("[PC|%s] Can't open Selection UI. Player State is INVALID"), *MainPlayerState->GetPersistantId())
	}
}

void AMainPlayerController::Request_Respawn()
{
	if (HasAuthority())
	{
		Respawn();
	} else
	{
		Server_Respawn();
	}
}

void AMainPlayerController::ToggleMainUI(bool bShow)
{
	ESlateVisibility TargetVisibility = bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden;

	if (PlayerHUD)
	{
		PlayerHUD->SetVisibility(TargetVisibility);
	}

	if (ChattingPanel)
	{
		ChattingPanel->SetVisibility(TargetVisibility);
	}
}

void AMainPlayerController::Request_SavePlayer()
{
	if (HasAuthority())
	{
		SavePlayer();
	} else
	{
		Server_SavePlayer(); // <--- 서버 RPC 호출로 변경!
	}
}

void AMainPlayerController::Server_SavePlayer_Implementation()
{
	SavePlayer();
}

void AMainPlayerController::SavePlayer()
{
	if (AMainGameMode* GM = Cast<AMainGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (AMainPlayerState* PS = GetPlayerState<AMainPlayerState>())
		{
			UE_LOG(LogTemp, Warning, TEXT("[PLAYER CONTROLLER] SAVE PLAYER"))
			GM->SavePlayer(PS);
		}	
	}
}

void AMainPlayerController::Client_SetViewTargetWithBlend_Implementation(AActor* NewTarget, float BlendTime)
{
	if (!IsValid(NewTarget))
	{
		FTimerHandle RetryHandle;
		TWeakObjectPtr<AMainPlayerController> WeakThis(this);
		TWeakObjectPtr<AActor> WeakTarget(NewTarget);
		GetWorldTimerManager().SetTimer(RetryHandle, [WeakThis, WeakTarget, BlendTime]()
			{
				if (WeakThis.IsValid() && WeakTarget.IsValid())
				{
					WeakThis->Client_SetViewTargetWithBlend(WeakTarget.Get(), BlendTime);
				}
			}, 0.1f, false);
		return;
	}

	this->bAutoManageActiveCameraTarget = false;
	this->SetViewTargetWithBlend(NewTarget, BlendTime);

	EnterSpectateMode();
}

void AMainPlayerController::Server_Respawn_Implementation()
{
	Respawn();
}

void AMainPlayerController::Client_RoleDeny_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Role Selection Denied"))
	if (RoleSelectionWidget)
	{
		RoleSelectionWidget->PlayDenyAlarm();
	}
}

void AMainPlayerController::Client_SetInputModeGame_Implementation()
{
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}

void AMainPlayerController::Server_ConfirmRole_Implementation(ECharacterRole NewRole)
{
	AMultiGameMode* GM = Cast<AMultiGameMode>(GetWorld()->GetAuthGameMode());
	AMainPlayerState* PS = Cast<AMainPlayerState>(PlayerState);
	AMainGameState* GS = Cast<AMainGameState>(GetWorld()->GetGameState());
	
	if (GM->CheckRoleAvailable(NewRole))
	{
		PS->SetPlayerRole(NewRole);
		UE_LOG(LogTemp, Warning, TEXT("Check Role %d in Server"), PS->GetPlayerRole());
		
		if (GS)
		{
			GS->RefreshSelectedRoles();
		}
		
		GM->RestartPlayer(this);
		Client_EndSelection();
		
	} else
	{
		Client_RoleDeny();
	}
}

void AMainPlayerController::Client_OpenSelectionUI_Implementation()
{	
	URoleSelection* SelectionWidget = CreateWidget<URoleSelection>(this, RoleSelectionWidgetClass);
	RoleSelectionWidget = SelectionWidget;
	
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(SelectionWidget->TakeWidget());
	SetInputMode(InputMode);
	bShowMouseCursor = true;
	
	SelectionWidget->AddToViewport();	
}

void AMainPlayerController::AddChat(FChattingData NewChattingData)
{
	if (!ChattingPanel) return;
	
	ChattingPanel->AddChatting(NewChattingData);
}

void AMainPlayerController::OpenDeathScreen()
{
	if (DeathScreenWidget) return;
	
	if (DeathScreenWidgetClass)
	{
		DeathScreenWidget = CreateWidget<UDeathScreen>(this, DeathScreenWidgetClass);
	
		bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(DeathScreenWidget->TakeWidget());
		SetInputMode(InputMode);
	
		DeathScreenWidget->AddToViewport();
	}
}

void AMainPlayerController::OnCloseDeathScreen()
{
	DeathScreenWidget = nullptr;
	bShowMouseCursor = false;
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
}

void AMainPlayerController::OnResume()
{
	HidePauseMenu();
}

void AMainPlayerController::OnSetting()
{
	UE_LOG(LogTemp, Warning, TEXT("SETTING BUTTON CLICKED"));

	if (SettingsWidgetClass)
	{
		SettingsWidget = CreateWidget<USettingsWidget>(this, SettingsWidgetClass);
		SettingsWidget->AddToViewport(11);
	}
}

void AMainPlayerController::OnQuit()
{
	UE_LOG(LogTemp, Warning, TEXT("QUIT BUTTON CLICKED"));

	//JWY-Quit 버튼 연타로 세션 정리와 맵 이동이 중복 실행되면 NetDriver 정리 순서가 꼬일 수 있어 한 번만 처리
	if (bIsQuitting)
	{
		return;
	}

	bIsQuitting = true;
	HidePauseMenu();

	//JWY-싱글은 네트워크 연결 정리가 필요 없으므로 기존처럼 로컬에서 바로 메인 복귀 처리
	if (!IsMultiplayerSession())
	{
		ReturnToMainMenuLocal();
		return;
	}

	//JWY-호스트는 서버이므로 원격 클라이언트를 먼저 메인으로 보내고 잠깐 뒤 본인 세션을 정리
	if (HasAuthority())
	{
		ReturnConnectedClientsToMainMenu();
		GetWorldTimerManager().SetTimer(
			HostReturnToMainMenuTimerHandle,
			this,
			&AMainPlayerController::ReturnToMainMenuLocal,
			HostReturnToMainMenuDelay,
			false);
		return;
	}

	//JWY-멀티 클라이언트는 바로 OpenLevel하지 않고 서버가 Client RPC로 복귀를 지시하게 요청
	Server_RequestReturnToMainMenu();

	//JWY-서버 응답이 끊긴 예외 상황에서도 사용자가 게임 안에 갇히지 않도록 늦은 fallback을 둠
	GetWorldTimerManager().SetTimer(
		ClientReturnToMainMenuFallbackTimerHandle,
		this,
		&AMainPlayerController::ReturnToMainMenuLocal,
		ClientReturnToMainMenuFallbackDelay,
		false);
}

void AMainPlayerController::Server_RequestReturnToMainMenu_Implementation()
{
	//JWY-서버 쪽 PlayerController도 나가는 중 상태를 기록해 중복 요청을 무시하고 클라이언트 복귀 RPC만 보냄
	if (bIsQuitting)
	{
		return;
	}

	bIsQuitting = true;
	Client_ReturnToMainMenu();
}

void AMainPlayerController::Client_ReturnToMainMenu_Implementation()
{
	//JWY-서버/호스트가 지시한 정상 복귀 경로이므로 클라이언트는 세션 정리 후 메인 메뉴로 이동
	bIsQuitting = true;
	ReturnToMainMenuLocal();
}

void AMainPlayerController::ReturnToMainMenuLocal()
{
	//JWY-서버 RPC, fallback timer, 호스트 지연 timer가 겹쳐도 실제 메인 복귀는 한 번만 실행
	if (bHasStartedReturnToMainMenu)
	{
		return;
	}

	bHasStartedReturnToMainMenu = true;
	GetWorldTimerManager().ClearTimer(ClientReturnToMainMenuFallbackTimerHandle);
	GetWorldTimerManager().ClearTimer(HostReturnToMainMenuTimerHandle);

	HidePauseMenu();
	if (SettingsWidget)
	{
		SettingsWidget->RemoveFromParent();
		SettingsWidget = nullptr;
	}

	bShowMouseCursor = false;
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	if (UMainGameInstance* GI = Cast<UMainGameInstance>(GetGameInstance()))
	{
		//JWY-BP_MainGameInstance에서 DestroySession 성공/실패 후 StartMap 이동을 연결할 수 있도록 공통 이벤트 호출
		GI->DestroySessionAndReturnToMainMenu();
		return;
	}

	//JWY-GameInstance를 못 찾는 예외 상황에서도 최소한 메인 메뉴로 복귀하도록 기존 SoftObject 이동을 fallback으로 유지
	UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), MainMenuLevel);
}

void AMainPlayerController::ReturnConnectedClientsToMainMenu()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	//JWY-호스트 종료 시 원격 클라이언트가 서버 종료를 갑자기 맞지 않도록 먼저 메인 복귀 RPC를 전송
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		AMainPlayerController* PC = Cast<AMainPlayerController>(It->Get());
		if (!PC || PC == this || PC->IsLocalController())
		{
			continue;
		}

		PC->Client_ReturnToMainMenu();
	}
}

bool AMainPlayerController::IsMultiplayerSession() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	//JWY-Standalone이 아니면 클라이언트/리스닝 서버 모두 네트워크 Quit 정리 흐름을 타야 함
	return World->GetNetMode() != NM_Standalone;
}

void AMainPlayerController::Respawn()
{
	if (AMultiGameMode* MGM = GetWorld()->GetAuthGameMode<AMultiGameMode>())
	{
		UE_LOG(LogTemp, Warning, TEXT("[MPC] MULTI RESPAWN"))
		MGM->HandlePlayerDeath(this);
		return;
	}
	
	if (AMainGameMode* SGM = GetWorld()->GetAuthGameMode<AMainGameMode>())
	{
		UE_LOG(LogTemp, Warning, TEXT("[MPC] SINGLE RESPAWN"))
		SGM->HandlePlayerDeath(this);
	}
}

void AMainPlayerController::EnterSpectateMode()
{
	bIsSpectating = true;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(SpectateInputMappingContext, 2);
	}
}

void AMainPlayerController::ExitSpectateMode()
{
	bIsSpectating = false;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->RemoveMappingContext(SpectateInputMappingContext);
	}
	this->bAutoManageActiveCameraTarget = true; // 카메라 자동 관리 다시 활성화

	if (APawn* MyPawn = GetPawn())
	{
		SetViewTargetWithBlend(MyPawn, 0.5f);
	}

	// 관전용 카메라 액터 제거
	if (SpectatorCamera)
	{
		SpectatorCamera->Destroy();
		SpectatorCamera = nullptr;
	}
}

void AMainPlayerController::SwitchSpectateTarget()
{
	if (!bIsSpectating) return;
	Server_RequestNextSpectateTarget();
}

void AMainPlayerController::Server_RequestNextSpectateTarget_Implementation()
{
	// MoonlightInfectionSystem에서 다음 타겟 찾기
	AMoonlightInfectionSystem* System = Cast<AMoonlightInfectionSystem>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AMoonlightInfectionSystem::StaticClass()));

	if (System)
	{
		System->SwitchSpectateTarget(this);
	}
}

void AMainPlayerController::Client_EnterSpectateMode_Implementation()
{
	EnterSpectateMode();
}

void AMainPlayerController::Client_ExitSpectateMode_Implementation()
{
	ExitSpectateMode();
}

void AMainPlayerController::Client_RestoreHUDAfterTransform_Implementation()
{
	// OnUnPossess에서 PlayerHUD가 파괴되므로 복귀 후 재생성 필요
	AMainPlayer* RestoredPlayer = Cast<AMainPlayer>(GetPawn());
	if (!RestoredPlayer) return;

	// PlayerHUD 재생성
	SetPlayerHUD(RestoredPlayer);

	// 인벤토리 컴포넌트 갱신 (UI 브로드캐스트)
	if (UInventoryComponent* Inv = RestoredPlayer->FindComponentByClass<UInventoryComponent>())
	{
		Inv->InventoryChanged();
	}
}

void AMainPlayerController::Client_SetSpectateTarget_Implementation(AActor* TargetPlayer)
{
	if (!IsValid(TargetPlayer))
	{
		// 리플리케이션 대기 후 재시도
		FTimerHandle RetryHandle;
		TWeakObjectPtr<AMainPlayerController> WeakThis(this);
		TWeakObjectPtr<AActor> WeakTarget(TargetPlayer);
		GetWorldTimerManager().SetTimer(RetryHandle, [WeakThis, WeakTarget]()
			{
				if (WeakThis.IsValid() && WeakTarget.IsValid())
				{
					WeakThis->Client_SetSpectateTarget(WeakTarget.Get());
				}
			}, 0.1f, false);
		return;
	}

	FVector SpawnLoc = TargetPlayer->GetActorLocation();

	if (ACharacter* TargetChar = Cast<ACharacter>(TargetPlayer))
	{
		if (USkeletalMeshComponent* Mesh = TargetChar->GetMesh())
		{
			if (Mesh->DoesSocketExist(TEXT("headSocket")))
			{
				SpawnLoc = Mesh->GetSocketLocation(TEXT("headSocket"));
			}
		}
	}

	if (!SpectatorCamera)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		SpectatorCamera = GetWorld()->SpawnActor<ASpectatorCameraActor>(
			ASpectatorCameraActor::StaticClass(),
			SpawnLoc,
			TargetPlayer->GetActorRotation(),
			Params
		);
	}

	if (SpectatorCamera)
	{
		SpectatorCamera->TargetActor = TargetPlayer;

		SpectatorCamera->SetActorLocation(SpawnLocation);

		this->bAutoManageActiveCameraTarget = false;
		this->SetViewTargetWithBlend(SpectatorCamera, 0.2f);
	}

	EnterSpectateMode();
}
