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
#include "Widgets/BaseButton.h"
#include "Widgets/PlayerHUD.h"
#include "Widgets/MainMenu/PauseMenu.h"
#include "Widgets/RoleSelection/RoleSelection.h"
#include "Widgets/DeathScreen.h"

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	HUD = Cast<AMainHUD>(GetHUD());
	
	if (ChattingPanelClass && IsLocalController())
	{
		ChattingPanel = CreateWidget<UChattingPanel>(this, ChattingPanelClass);
		ChattingPanel->AddToViewport();
	}
	
	if (PauseWidgetClass && IsLocalController())
	{
		PauseMenu = CreateWidget<UPauseMenu>(this, PauseWidgetClass);
		
		PauseMenu->ResumeButton->OnClicked.AddDynamic(this, &AMainPlayerController::OnResume);
		PauseMenu->SettingButton->OnClicked.AddDynamic(this, &AMainPlayerController::OnSetting);
		PauseMenu->QuitButton->OnClicked.AddDynamic(this, &AMainPlayerController::OnQuit);
		
		PauseMenu->AddToViewport(10);
		HidePauseMenu();
	}
	
	MainGameInstance = Cast<UMainGameInstance>(GetGameInstance());
	MainGameMode = Cast<AMainGameMode>(GetWorld()->GetAuthGameMode());
	MainPlayerState = GetPlayerState<AMainPlayerState>();
	MainGameState = Cast<AMainGameState>(GetWorld()->GetGameState());
	
	if (HasAuthority()&&MainPlayerState&&MainGameState->IsMulti)
	{
		if (MainPlayerState->GetPlayerRole() == ECharacterRole::NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("Open Selection UI in Player Controller"))
			Client_OpenSelectionUI();
		} else
		{
			UE_LOG(LogTemp, Warning, TEXT("[PC] Can't open Selection UI. Player role is %d"), MainPlayerState->GetPlayerRole())
		}
	}
}

void AMainPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// 향상된 입력 컴포넌트
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(ChatAction, ETriggerEvent::Started, this, &AMainPlayerController::ToggleChatMode);
		EnhancedInputComponent->BindAction(ESCAction, ETriggerEvent::Started, this, &AMainPlayerController::TogglePause);
	}
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InputMappingContext, 1);
	}
}

void AMainPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AMainPlayerController::OnUnPossess()
{
	UE_LOG(LogTemp, Warning, TEXT("UNPOSSESSED"));
	if (IsLocalController() && PlayerHUD)
	{
		PlayerHUD->RemoveFromParent();
		PlayerHUD = nullptr;
	}
	
	Super::OnUnPossess();
}

void AMainPlayerController::SetPlayerHUD(AMainPlayer* OwnerPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] Possessed in %s"), *GetName(), *OwnerPlayer->GetName())
	AMainPlayer* InPlayer = Cast<AMainPlayer>(OwnerPlayer);
	if (InPlayer && HUDClass && IsLocalController())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%hs]Set Player HUD"), HasAuthority()?"SERVER":"CLIENT")
		PlayerHUD = CreateWidget<UPlayerHUD>(this, HUDClass);
		PlayerHUD->SetPlayerRef(InPlayer);
		InPlayer->SetHUDWidget(PlayerHUD);
		PlayerHUD->AddToViewport();
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
	IsPause = !Cast<AMultiGameMode>(GetWorld()->GetAuthGameMode());
	UE_LOG(LogTemp, Warning, TEXT("[GAMEMODE MULTI?] : %d"), IsPause)
	bShowMouseCursor = true;
	
	FInputModeUIOnly Mode;
	SetInputMode(Mode);
	
	UGameplayStatics::SetGamePaused(GetWorld(), true);
	
	PauseMenu->SetVisibility(ESlateVisibility::Visible);
}

void AMainPlayerController::HidePauseMenu()
{
	IsPause = false;
	bShowMouseCursor = false;
	
	FInputModeGameOnly Mode;
	SetInputMode(Mode);
	
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	
	PauseMenu->SetVisibility(ESlateVisibility::Collapsed);
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
	RoleSelectionWidget->RemoveFromParent();
}

void AMainPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	MainPlayerState = GetPlayerState<AMainPlayerState>();
	
	if (MainPlayerState)
	{
		if (MainPlayerState->GetPlayerRole() == ECharacterRole::NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("[PC|%s] Open Selection UI in Player Controller"), *GetPlayerState<AMainPlayerState>()->GetPersistantId())
			Client_OpenSelectionUI();
		} else
		{
			UE_LOG(LogTemp, Warning, TEXT("[PC|%s] Can't open Selection UI. Player role is %d"), *GetPlayerState<AMainPlayerState>()->GetPersistantId(), MainPlayerState->GetPlayerRole())
		}
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PC|%s] Can't open Selection UI. Player State is INVALID"), *GetPlayerState<AMainPlayerState>()->GetPersistantId())
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
	
	if (GM->CheckRoleAvailable(NewRole))
	{
		PS->SetPlayerRole(NewRole);
		UE_LOG(LogTemp, Warning, TEXT("Check Role %d in Server"), PS->GetPlayerRole());
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
}

void AMainPlayerController::OnQuit()
{
	UE_LOG(LogTemp, Warning, TEXT("QUIT BUTTON CLICKED"));
	
	UGameplayStatics::OpenLevelBySoftObjectPtr(GetWorld(), MainMenuLevel);
}

void AMainPlayerController::Respawn()
{
	AMainGameMode* GM = GetWorld()->GetAuthGameMode<AMainGameMode>();
	GM->HandlePlayerDeath(this);
}