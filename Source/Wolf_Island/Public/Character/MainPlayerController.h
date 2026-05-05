// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputMappingContext.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "Games/MainGameState.h"
#include "Games/MainPlayerState.h"
#include "Widgets/Chatting/ChattingPanel.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */

class UDeathScreen;
class URoleSelection;
class UMainGameInstance;
class AMainGameMode;
class UPauseMenu;
class UPlayerHUD;

UCLASS()
class WOLF_ISLAND_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputMappingContext* InputMappingContext;

	// 관전 모드
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input|Spectate")
	UInputMappingContext* SpectateInputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input|Spectate")
	UInputAction* SpectateNextAction;
	//

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* ChatAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* ESCAction;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category="Widget")
	class AMainHUD* HUD;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category="Widget")
	UPlayerHUD* PlayerHUD;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category="Widget")
	UChattingPanel* ChattingPanel;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category="Widget")
	UPauseMenu* PauseMenu;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category="Widget")
	URoleSelection* RoleSelectionWidget;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category="Widget")
	UDeathScreen* DeathScreenWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UPlayerHUD> HUDClass;

	UPROPERTY(EditDefaultsOnly, Category="Widget")
	TSubclassOf<UChattingPanel> ChattingPanelClass;

	UPROPERTY(EditDefaultsOnly, Category="Widget")
	TSubclassOf<UPauseMenu> PauseWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	TSubclassOf<class UUserWidget> FishTrapScreenClass;

	UPROPERTY(EditDefaultsOnly, Category="Widget")
	TSubclassOf<URoleSelection> RoleSelectionWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="Widget")
	TSubclassOf<UDeathScreen> DeathScreenWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UMainGameInstance* MainGameInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AMainGameMode* MainGameMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AMainPlayerState* MainPlayerState;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AMainGameState* MainGameState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSoftObjectPtr<UWorld> MainMenuLevel;

	UPROPERTY(BlueprintReadOnly)
	bool IsChat = false;

	UPROPERTY(BlueprintReadOnly)
	bool IsPause = false;

	// JWY - Quit 버튼이 여러 번 눌려 클라이언트/세션 정리가 중복 실행되는 것을 막기 위한 상태값입니다.
	UPROPERTY(BlueprintReadOnly)
	bool bIsReturningToMainMenu = false;

	bool bHasStartedReturnToMainMenuTravel = false;

	FTimerHandle ReturnToMainMenuTimerHandle;

	// 관전
	void EnterSpectateMode();
	void ExitSpectateMode();
	void SwitchSpectateTarget();

	UFUNCTION(Server, Reliable)
	void Server_RequestNextSpectateTarget();

	bool bIsSpectating = false;
	
	UFUNCTION(Client, Reliable)
	void Client_EnterSpectateMode();

	UFUNCTION(Client, Reliable)
	void Client_ExitSpectateMode();

	UFUNCTION(Client, Reliable)
	void Client_SetSpectateTarget(AActor* TargetPlayer);

	UPROPERTY()
	class ASpectatorCameraActor* SpectatorCamera;
	//

	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

	virtual void OnUnPossess() override;

	virtual void SetupInputComponent() override;
	
	virtual void InitPlayerState() override;

	UFUNCTION(BlueprintCallable)
	void SetPlayerHUD(AMainPlayer* OwnerPlayer);

	UFUNCTION(BlueprintCallable)
	void ToggleChatMode();

	UFUNCTION(BlueprintCallable)
	void TogglePause();

	UFUNCTION()
	void EnterChatMode();
	UFUNCTION()
	void ExitChatMode();

	UFUNCTION()
	void DisplayPauseMenu();

	UFUNCTION()
	void HidePauseMenu();

	UFUNCTION(Client, Reliable)
	void Client_OpenFishTrapUI(class AFishTrap* TargetTrap, class AActor* Interactor);

	UFUNCTION(BlueprintCallable)
	void AddChat(FChattingData NewChattingData);

	UFUNCTION(BlueprintCallable)
	void OpenDeathScreen();

	UFUNCTION(BlueprintCallable)
	void OnCloseDeathScreen();

	UFUNCTION(BlueprintCallable)
	void OnResume();
	UFUNCTION(BlueprintCallable)
	void OnSetting();
	UFUNCTION(BlueprintCallable)
	void OnQuit();

	// JWY - 클라이언트가 직접 맵을 열지 않고 서버에게 먼저 나가기 의사를 전달하도록 하는 RPC입니다.
	UFUNCTION(Server, Reliable)
	void Server_RequestReturnToMainMenu();

	// JWY - 서버가 클라이언트에게 안전하게 메인 메뉴 복귀를 지시할 때 사용하는 공통 Client RPC입니다.
	UFUNCTION(Client, Reliable)
	void Client_ReturnToMainMenu();

	UFUNCTION(BlueprintCallable)
	void ReturnToMainMenuLocal();

	UFUNCTION(BlueprintCallable)
	void ReturnConnectedClientsToMainMenu();

	bool IsMultiplayerSession() const;

	//플레이어 사망 후 리스폰 버튼 클릭 시 실행할 리스폰 시퀀스
	UFUNCTION(BlueprintCallable)
	void Respawn();

	//멀티 플레이 코드
	UFUNCTION(BlueprintCallable)
	void Request_SendChat(FChattingData NewChattingData);

	UFUNCTION(Server, Reliable)
	void Server_SendChat(FChattingData NewChattingData);

	UFUNCTION()
	void SendChat(FChattingData NewChattingData);

	//역할 선택 관련
	UFUNCTION(Client, Reliable)
	void Client_OpenSelectionUI();

	UFUNCTION(Server, Reliable)
	void Server_ConfirmRole(ECharacterRole NewRole);

	UFUNCTION(Client, Reliable)
	void Client_SetInputModeGame();

	UFUNCTION(Client, Reliable)
	void Client_RoleDeny();

	UFUNCTION(Client, Reliable)
	void Client_EndSelection();

	virtual void OnRep_PlayerState() override;

	//사망 시 부활 요청
	UFUNCTION(BlueprintCallable)
	void Request_Respawn();

	UFUNCTION(Server, Reliable)
	void Server_Respawn();


	// 관전 전용 카메라 시점 함수
	// 클라이언트에서 실행될 RPC 선언
	UFUNCTION(Client, Reliable)
	void Client_SetViewTargetWithBlend(AActor* NewTarget, float BlendTime);

	// 플레이어 UI 끄고 키는 함수
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleMainUI(bool bShow);
};
