// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputMappingContext.h"
#include "GameFramework/PlayerController.h"
#include "Games/MainGameState.h"
#include "Games/MainPlayerState.h"
#include "Widgets/Chatting/ChattingPanel.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */

class USettingsWidget;
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
	USettingsWidget* SettingsWidget;

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
	
	UPROPERTY(EditDefaultsOnly, Category="Widget")
	TSubclassOf<USettingsWidget> SettingsWidgetClass;

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

	//JWY-Quit 버튼 연타로 서버 RPC/세션 정리/맵 이동이 중복 실행되지 않도록 막기 위한 플래그
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Network|Quit")
	bool bIsQuitting = false;

	//JWY-서버 RPC 응답과 fallback이 동시에 들어와도 실제 메인 복귀 로직은 한 번만 실행되도록 막기 위한 플래그
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Network|Quit")
	bool bHasStartedReturnToMainMenu = false;

	//JWY-호스트가 원격 클라이언트에게 메인 복귀 RPC를 보낸 뒤 본인이 세션을 정리하기 전 잠깐 기다리는 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Network|Quit")
	float HostReturnToMainMenuDelay = 0.5f;

	//JWY-클라이언트가 서버에 Quit 요청을 보냈는데 응답을 못 받는 예외 상황에서 로컬 복귀를 보장하기 위한 대기 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Network|Quit")
	float ClientReturnToMainMenuFallbackDelay = 3.0f;

	FTimerHandle HostReturnToMainMenuTimerHandle;
	FTimerHandle ClientReturnToMainMenuFallbackTimerHandle;

	UPROPERTY(BlueprintReadOnly)
	bool IsChat = false;

	UPROPERTY(BlueprintReadOnly)
	bool IsPause = false;

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

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

	//사망화면 위젯을 확실히 제거하고 게임 입력으로 복구
	UFUNCTION(BlueprintCallable)
	void CloseDeathScreen();

	UFUNCTION(BlueprintCallable)
	void OnResume();
	UFUNCTION(BlueprintCallable)
	void OnSetting();
	UFUNCTION(BlueprintCallable)
	void OnQuit();

	//JWY-클라이언트가 인게임 Quit을 누르면 바로 OpenLevel하지 않고 서버가 정리 순서를 잡도록 요청하기 위해 추가
	UFUNCTION(Server, Reliable)
	void Server_RequestReturnToMainMenu();

	//JWY-서버/호스트가 클라이언트에게 안전하게 메인 메뉴로 돌아가라고 지시하기 위해 추가
	UFUNCTION(Client, Reliable)
	void Client_ReturnToMainMenu();

	//JWY-싱글/멀티 공통으로 UI를 정리하고 GameInstance의 세션 정리 흐름을 호출하기 위해 추가
	UFUNCTION(BlueprintCallable, Category="Network|Quit")
	void ReturnToMainMenuLocal();

	//JWY-호스트가 나갈 때 원격 클라이언트들을 먼저 메인 메뉴로 돌려보내기 위해 추가
	void ReturnConnectedClientsToMainMenu();

	//JWY-현재 플레이가 네트워크 세션인지 확인해 싱글과 멀티 Quit 흐름을 분리하기 위해 추가
	UFUNCTION(BlueprintCallable, Category="Network|Quit")
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

	// 늑대 변신 복귀 후 PlayerHUD 재생성 + 인벤토리 UI 갱신
	UFUNCTION(Client, Reliable)
	void Client_RestoreHUDAfterTransform();

	// 플레이어 UI 끄고 키는 함수
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleMainUI(bool bShow);
	
	//플레이어 데이터 저장 요첨
	UFUNCTION()
	void Request_SavePlayer();
	UFUNCTION(Server, Reliable)
	void Server_SavePlayer();
	UFUNCTION()
	void SavePlayer();
};
