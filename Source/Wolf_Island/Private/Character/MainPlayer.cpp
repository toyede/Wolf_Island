// Fill out your copyright notice in the Description page of Project Settings.


#include "Wolf_Island/Public/Character/MainPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "MaterialHLSLTree.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Craft/BonFireUI.h"
#include "Widgets/Craft/RepairUI.h"
#include "Components/StatusComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/WeaponComponent.h"
#include "Item/Tree.h"
#include "Components/InventoryComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interaction/InteractionInterface.h"
#include "Item/Pickup.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/PlayerHUD.h"
#include "BuoyancyComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BillboardComponent.h"
#include "WaterBodyComponent.h"
#include "Actors/RespawnableFoliage.h"
#include "Character/MainPlayerController.h"
#include "Character/Torch.h"
#include "Components/BuildingComponent.h"
#include "Components/WidgetComponent.h"
#include "Games/MainSaveGame.h"
#include "Games/GameModes/MainGameMode.h"
#include "Moon/MoonlightInfectionSystem.h"
#include "Games/MainPlayerState.h"
#include "Games/MainGameState.h"
#include "AI/Sense/AISense_Scent.h"
#include "Widgets/Inventory/Inventory.h"


void AMainPlayer::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	UE_LOG(LogTemp, Warning, TEXT("[PLAYER] POSSESSED BY"));
}

void AMainPlayer::PawnClientRestart()
{
	UE_LOG(LogTemp, Warning, TEXT("[PLAYER] PawnClientRestart Called"))
	Super::PawnClientRestart();
	
	if (IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PLAYER] Restore Color Saturation"))
		FirstPersonCamera->PostProcessSettings.bOverride_ColorSaturation = true;
		FirstPersonCamera->PostProcessSettings.ColorSaturation = FVector4(1, 1,1,1);
		
		if (AMainPlayerController* MainController = Cast<AMainPlayerController>(GetController()))
		{
			MainController->SetPlayerHUD(this);
		} 
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("HAS NO CONTROLLER"))
		}	
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("NOT LOCALLY CONTROLLED"))
	}
}

void AMainPlayer::Restart()
{
	Super::Restart();
}

void AMainPlayer::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	UE_LOG(LogTemp, Warning, TEXT("[PLAYER] OnRep_PlayerState : %s"), *GetName());
}

void AMainPlayer::OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
	UE_LOG(LogTemp, Warning, TEXT("[PLAYER] On PlayerState Changed : %s"), *GetName());
	Super::OnPlayerStateChanged(NewPlayerState, OldPlayerState);
	
	if (AMainGameMode* GM = GetWorld()->GetAuthGameMode<AMainGameMode>())
	{
		GM->AfterRestartPlayer(GetController(), false);
	}
	
	if (NickName)
	{
		if (UNickName* NickNameWidget = Cast<UNickName>(NickName->GetWidget()))
		{
			//NickNameWidget->UpdateName(NewPlayerState);
		}
	}

	if (AMainPlayerState* PS = GetPlayerState<AMainPlayerState>())
	{
		for (const FName& RecipeID : DefaultRecipes)
		{
			if (RecipeID != NAME_None && RecipeID.ToString() != TEXT("None"))
			{
				PS->UnlockPersonalRecipe(RecipeID);
			}
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("[PLAYER] Player State Changed. OLD[%s] -> NEW[%s]"), *OldPlayerState->GetName(), *NewPlayerState->GetName())
}

// Sets default values
AMainPlayer::AMainPlayer()
{
	UE_LOG(LogTemp, Warning, TEXT("[PLAYER] Construct"))
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StatusComponent = CreateDefaultSubobject<UStatusComponent>("StatusComponent");

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("InventoryComponent");

	WeaponComponent = CreateDefaultSubobject<UWeaponComponent>("WeaponComponent");
	
	BuoyancyComponent = CreateDefaultSubobject<UBuoyancyComponent>("BuoyancyComponent");
	
	BuildingComponent = CreateDefaultSubobject<UBuildingComponent>("BuildingComponent");
	
	WaterLevelCheckPoint = CreateDefaultSubobject<UBillboardComponent>("WaterLevelCheckPoint");
	
	WaterAmbience = CreateDefaultSubobject<UAudioComponent>("WaterAmbience");
	
	NickName = CreateDefaultSubobject<UWidgetComponent>("NickNameWidget");

	//손에 든 아이템 메쉬
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>("Item");
	//손 소켓에 부-착!
	ItemMesh->SetupAttachment(GetMesh(), "hand_r");
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	GetMesh()->SetRelativeTransform(
		FTransform(
			FRotator(0, -90, 0),
			FVector(0,0,-90)
			));
	/**/
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>("FirstPersonCamera");
	
	//메시에 카메라 붙이기
	//FirstPersonCamera->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "headSocket");
	FirstPersonCamera->SetupAttachment(GetMesh(), "headSocket");
	//컨트롤러 마우스 위치 입력을 카메라 입력에 반영
	FirstPersonCamera->bUsePawnControlRotation = true;
	
	FirstPersonCamera->SetRelativeTransform(
		FTransform(
			FRotator(0, 90, -90),
			FVector(0,10,0)
			));
		
	//인벤토리 초기화
	InventoryComponent->SetSlotsCapacity(30);
	InventoryComponent->SetWeightCapacity(100);

	GetCharacterMovement()->SetIsReplicated(true);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	
	//수영을 위한 부력 컴포넌트 세팅
	WaterLevelCheckPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
	BuoyancyComponent->AddCustomPontoon(25.0f, WaterLevelCheckPoint->GetRelativeLocation());
	
	NickName->SetupAttachment(GetMesh());
	//NickName->bOwnerNoSee = true;
	
	GetCharacterMovement()->MaxFlySpeed = SwimmingSpeed;
}

// Called when the game starts or when spawned
void AMainPlayer::BeginPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("[PLAYER] BeginPlay"))
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		
		TArray<AActor*> Found;
		UGameplayStatics::GetAllActorsOfClass(
			GetWorld(), AMoonlightInfectionSystem::StaticClass(), Found);
		if (Found.Num() > 0)
		{
			AMoonlightInfectionSystem* System =
				Cast<AMoonlightInfectionSystem>(Found[0]);
			TArray<AActor*> Self;
			Self.Add(this);
			System->BindPlayers(Self);
		}
	}

	InteractableData.InteractionDuration = InteractionDuration;
	
	if (IsLocallyControlled())
	{
		NickName->SetVisibility(false);
	}
	
	if(StatusComponent){
		//서버에서 실행
		if (HasAuthority())
		{
			//상태 델리게이트 바인딩
			StatusComponent->OnStaminaZero.AddUniqueDynamic(this, &AMainPlayer::Request_StopRun);

			//죽음 바인딩
			StatusComponent->OnHPZero.AddUniqueDynamic(this, &AMainPlayer::OnDeath);
			
			//배고픔, 수분 감소 시작
			StatusComponent->StartHunger();
			StatusComponent->StartHydration();

			//강제 휴식 애님 바인딩 (서버에서 브로드캐스트되므로 서버에서만 바인딩)
			StatusComponent->OnForcedRestStart.AddUniqueDynamic(this, &AMainPlayer::Multi_PlayForcedRestStart);
			StatusComponent->OnForcedRestEnd.AddUniqueDynamic(this, &AMainPlayer::Multi_PlayForcedRestEnd);
		}
		
		//산소 게이지 숨기기 바인딩(테스트용)
		StatusComponent->OnAirFull.AddUniqueDynamic(this, &AMainPlayer::HideAirBar);
	}

	if (InventoryComponent)
	{
		//아이템 업데이트 바인딩
		InventoryComponent->OnInventoryUpdated.AddUObject(this, &AMainPlayer::RefreshHand);
		
		//무게 업데이트 바인딩
		InventoryComponent->OnCurrentWeightChanged.AddUObject(this, &AMainPlayer::OnCurrentWeightChanged);
		
		
		if (IsLocallyControlled() && InventoryWidgetClass)
		{
			InventoryWidget = CreateWidget<UInventory>(GetWorld(), InventoryWidgetClass);
			InventoryWidget->AddToViewport();
			InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (WeaponComponent)
	{
		RefreshHand();
	}
	
	if (BuoyancyComponent)
	{
		BuoyancyComponent->OnEnteredWaterDelegate.AddUniqueDynamic(this, &AMainPlayer::EnterWater);
		BuoyancyComponent->OnExitedWaterDelegate.AddUniqueDynamic(this, &AMainPlayer::ExitWater);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[%s] All Components Set"), *GetName());
	
	MainPlayerController = Cast<AMainPlayerController>(GetController());
	InteractableData.CanInteract = false;
	
	//토치 설정
	FActorSpawnParameters SpawnParams;
	Torch = GetWorld()->SpawnActor<ATorch>(TorchClass, SpawnParams);
	
	if (Torch)
	{
		Torch->AttachToComponent(
			GetMesh(), 
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			FName("hand_r"));
		Torch->SetActorHiddenInGame(true);
	}

	// 냄새 보고 타이머 설정
	GetWorldTimerManager().SetTimer(ScentTimerHandle, this, &AMainPlayer::ReportScent, ScentReportInterval, true);
	
	if (AMainPlayerController* PC = Cast<AMainPlayerController>(GetController()))
	{
		PC->Request_SavePlayer();
	}
}

void AMainPlayer::ReportScent()
{
	UAISense_Scent::ReportScentEvent(
		this,
		GetActorLocation(),
		ScentIntensity,
		ScentMaxRange,
		this // Instigator (나 자신)
	);
}

void AMainPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(LogTemp, Warning, TEXT("[PLAYER] END REASON : %s"), *StaticEnum<EEndPlayReason::Type>()->GetNameStringByValue((int)EndPlayReason));
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	
	// 인벤토리 위젯 바인딩 해제
	if (InventoryWidget)
	{
		InventoryWidget->RemoveFromParent();
		InventoryWidget = nullptr;
	}
	
	// 인벤토리 컴포넌트 바인딩 해제
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryUpdated.RemoveAll(this);
		InventoryComponent->OnCurrentWeightChanged.RemoveAll(this);
	}
	
	//바인딩 해제 코드
	if(StatusComponent){
		//서버에서 실행
		if (HasAuthority())
		{
			//상태 델리게이트 바인딩
			StatusComponent->OnStaminaZero.RemoveDynamic(this, &AMainPlayer::Request_StopRun);

			//죽음 바인딩
			StatusComponent->OnHPZero.RemoveDynamic(this, &AMainPlayer::OnDeath);

			//강제 휴식 애님 바인딩 해제
			StatusComponent->OnForcedRestStart.RemoveDynamic(this, &AMainPlayer::Multi_PlayForcedRestStart);
			StatusComponent->OnForcedRestEnd.RemoveDynamic(this, &AMainPlayer::Multi_PlayForcedRestEnd);
		}
		
		//산소 게이지 숨기기 바인딩(테스트용)
		StatusComponent->OnAirFull.RemoveDynamic(this, &AMainPlayer::HideAirBar);
	}
	
	if (BuoyancyComponent)
	{
		BuoyancyComponent->OnEnteredWaterDelegate.RemoveDynamic(this, &AMainPlayer::EnterWater);
		BuoyancyComponent->OnExitedWaterDelegate.RemoveDynamic(this, &AMainPlayer::ExitWater);
	}
	
	Super::EndPlay(EndPlayReason);
}

void AMainPlayer::Destroyed()
{
	UE_LOG(LogTemp, Warning, TEXT("[PLAYER] DESTROYED"));
	
	Super::Destroyed();
}

// Called every frame
void AMainPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (IsLocallyControlled()&&
		GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckFrequency)
	{
		CheckInteraction();
	}
	
	if (HasAuthority() && IsTracingAttack)
	{
		for (auto& TracePoint : TracePoints)
		{
			FVector Curr = TracePoint.Value.Source->GetSocketLocation(TracePoint.Key);
			WeaponTrace(TracePoint.Value.Prev, Curr);
			TracePoint.Value.Prev = Curr;
		}
	}
}

// Called to bind functionality to input
void AMainPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//IA와 IMC는 블루프린트에서 할당
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 향상된 입력 컴포넌트
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 점프
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started,
			this, &AMainPlayer::StartJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed,
			this, &AMainPlayer::StopJumping);

		// 이동
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered,
			this, &AMainPlayer::Move);
		
		//수영 오르내리기
		EnhancedInputComponent->BindAction(WaterElevationAction, ETriggerEvent::Triggered,
			this, &AMainPlayer::WaterElevation);
		
		// 시야
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered,
			this, &AMainPlayer::Look);

		//달리기
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Triggered,
			this, &AMainPlayer::Request_Run);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed,
			this, &AMainPlayer::Request_StopRun);

		//웅크리기
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started,
			this, &AMainPlayer::Request_ToggleCrouch);
		
		//인터랙션
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started,
			this, &AMainPlayer::BeginInteract);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed,
			this, &AMainPlayer::EndInteract);

		//인벤토리
		EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started,
			this, &AMainPlayer::ToggleInventory);

		//아이템 사용 - 좌클릭 꾹 누르기
		EnhancedInputComponent->BindAction(UseItemAction, ETriggerEvent::Started,
			this, &AMainPlayer::Request_StartUseItem);
		EnhancedInputComponent->BindAction(UseItemAction, ETriggerEvent::Completed,
			this, &AMainPlayer::Request_StopUseItem);

		//핫바 숫자키
		EnhancedInputComponent->BindAction(HotBarAction, ETriggerEvent::Triggered,
			this, &AMainPlayer::HandleHotBar); 
		//핫바 마우스 휠
		EnhancedInputComponent->BindAction(HotBarWheelAction, ETriggerEvent::Triggered,
			this, &AMainPlayer::HandleHotBarWithWheel);

		//공격
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started,
			this, &AMainPlayer::Request_Attack);
		
		//아이템 버리기
		EnhancedInputComponent->BindAction(DropItemAction, ETriggerEvent::Started,
			this, &AMainPlayer::DropItemOnHotBar);
		
		//감정표현
		EnhancedInputComponent->BindAction(EmotionAction, ETriggerEvent::Started,
			this, &AMainPlayer::Emotion);
	}
}

void AMainPlayer::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// 입력 매핑 컨텍스트(IMC) 추가
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
}

void AMainPlayer::OnCurrentWeightChanged()
{
	if (!InventoryComponent || !StatusComponent) return;
	
	float CurrentWeight = InventoryComponent->GetCurrentWeight();
	
	//- 100일 경우, 스테미나, 물, 배고픔의 소모량 50% 증가 및 이동속도 10% 감소
	if (CurrentWeight >= 100.0f)
	{
		StatusComponent->SetMultiplier(1.5f);
		MovementMultiplier = 0.9f;
	} 
	//- 75이상이면 스테미나, 물, 배고픔의 소모량 20% 증가
	else if (CurrentWeight >= 75.0f)
	{
		StatusComponent->SetMultiplier(1.2f);
		MovementMultiplier = 1.0f;
	} 
	//- 50이상이면 스테미나, 물, 배고픔의 소모량 10% 증가
	else if (CurrentWeight >= 50.0f)
	{
		StatusComponent->SetMultiplier(1.1f);
		MovementMultiplier = 1.0f;
	}
	//- 50미만이면 정상화
	else
	{
		StatusComponent->SetMultiplier(1.0f);
		MovementMultiplier = 1.0f;
	}
}

void AMainPlayer::StartJump()
{
	if (IsBuildingInputBlocked()) return;

	//스태미나가 0이면 점프 불가
	if (StatusComponent->CurrentStamina <= JumpConsumeAmount) return;

	//낙하 중(점프 중) 이면 점프 불가
	if (GetCharacterMovement()->IsFalling()) return;

	//슬라이딩 중이면 점프 불가 또는 기절 중이면
	if (IsSliding || IsInability) return;
	
	//앉아 있으면 일어서기
	if (HasAuthority())
	{
		if (IsCrouching)
		{
			//UE_LOG(LogTemp, Warning, TEXT("[JUMP] UNCROUCHED"));
			Request_ToggleCrouch();
		}
	
		//달리는 중 점프하면 스태미나 감소 중단
		if (IsRunning)
		{
			//UE_LOG(LogTemp, Warning, TEXT("[JUMP] STOP STAMINA"));
			StatusComponent->StopStamina();
		}

		//점프 시 스태미나 회복 중단
		//UE_LOG(LogTemp, Warning, TEXT("[JUMP] STOP STAMINA"));
		StatusComponent->StopRecoverStamina();
		//점프 스태미나 소모
		//UE_LOG(LogTemp, Warning, TEXT("[JUMP] CONSUME STAMINA"));
		StatusComponent->DecreaseStamina(JumpConsumeAmount);
	}
	
	if (JumpSound)
	{
		UE_LOG(LogTemp, Warning, TEXT("[JUMP] PLAY JUMPSOUND"));
		Multi_PlaySoundAtLocation(JumpSound, GetActorLocation());
	}

	Jump();
	UE_LOG(LogTemp, Warning, TEXT("[JUMP] JUMP EXECUTED"))
}

//착지 시 함수
void AMainPlayer::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	float FallForce = FMath::Abs(GetVelocity().Z);
	UE_LOG(LogTemp, Warning, TEXT("%f"), FallForce);

	if (FallForce > 1000.0f)
	{
		if (FallForce > 3000.0f)
		{
			UGameplayStatics::ApplyDamage(
				this, 
				FallForce*1.0f,
				GetController(),
				this,
				UDamageType::StaticClass());
		} else
		{
			UGameplayStatics::ApplyDamage(
				this, 
				FallForce*0.03f,
				GetController(),
				this,
				UDamageType::StaticClass());
		}
	}

	//달리는 중이면 스태미나 감소 시작
	if (HasAuthority())
	{
		if (IsRunning)
		{
			StatusComponent->StartStamina();
		} else
		{
			if(!GetWorld()->GetTimerManager().IsTimerActive(StatusComponent->StaminaRecoverTimer)){
				StatusComponent->StartRecoverStamina();
			}
		}
	}
}

//시야 함수
void AMainPlayer::Look(const FInputActionValue& Value)
{
	if (IsBuildingInputBlocked()) return;

	FVector2D LookAxisVector = Value.Get<FVector2D>();
	//UE_LOG(LogTemp, Warning, TEXT("LOOK X: %f, Y: %f"), LookAxisVector.X, LookAxisVector.Y);
	float sen = 1;

	if (Controller)
	{
		AddControllerYawInput(LookAxisVector.X * sen);
		AddControllerPitchInput(LookAxisVector.Y * sen);
	}
}

//이동 함수
void AMainPlayer::Move(const FInputActionValue& Value)
{
	if (IsBuildingInputBlocked()) return;

	FVector2D MovementVector = Value.Get<FVector2D>();
	//UE_LOG(LogTemp, Warning, TEXT("MOVE X: %f, Y: %f"), MovementVector.X, MovementVector.Y);

	if (Controller)
	{
		if (IsSwimming)
		{
			FVector ForwardVector = UKismetMathLibrary::GetForwardVector(GetControlRotation());
			AddMovementInput(GetActorRightVector(), 
				MovementVector.X * WaterDeceleration * MovementMultiplier);
			AddMovementInput(ForwardVector, 
				MovementVector.Y * WaterDeceleration * MovementMultiplier);
		} else
		{
			AddMovementInput(GetActorRightVector(), 
				MovementVector.X * MovementMultiplier);
			AddMovementInput(GetActorForwardVector(), 
				MovementVector.Y * MovementMultiplier);
		}
	}
}

//Shift 누른 상태로 Run -> 스태미나 소진 -> Run 상태 유지
//특정 시간 후 스태미나 회복 -> Shift 떼면 스태미나 소진
void AMainPlayer::Run()
{	
	if (IsBuildingInputBlocked()) return;

	//속도가 있는가? -> 뛰는 중인가?
	if (GetVelocity().Size() > 0){

		//낙하 중이면 달리기 불가 또는 기절 중이면
		if (GetMovementComponent()->IsFalling() || IsInability)
		{
			if(IsRunning){
				//스태미나 감소 중단
				StatusComponent->StopStamina();
				StatusComponent->StartRecoverStamina();
				IsRunning = false;
			}
			return;
		}
		
		if(!IsRunning){
			//스태미나 0이거나 웅크리는 중이면
			if (StatusComponent->CurrentStamina <= 0 || IsCrouching) return;
	
			GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
			GetCharacterMovement()->MaxFlySpeed = SwimmingSprintSpeed;
			StatusComponent->StopStamina();

			//이동 속도가 0 초과일 때만 스태미나 감소
			if (GetVelocity().Size() > 0)
			{
				StatusComponent->StartStamina();
			}
	
			IsRunning = true;
		}

	} else {

		if(IsRunning){
			//스태미나 감소 중단
			StatusComponent->StopStamina();
			StatusComponent->StartRecoverStamina();
			IsRunning = false;
		}

	}
}

void AMainPlayer::StopRun()
{
	//달리기 중일 때만 달리기 중지 시퀀스 작동
	if (IsRunning)
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		GetCharacterMovement()->MaxFlySpeed = SwimmingSpeed;
		//스태미나 감소 중단
		StatusComponent->StopStamina();
		//스태미나 회복 타이머가 실행 중이 아니면
		if (!GetWorld()->GetTimerManager().TimerExists(StatusComponent->StaminaRecoverTimer))
		{
			//스태미나 회복 타이머 실행
			StatusComponent->StartRecoverStamina();
		}
		IsRunning = false;
	}
}

void AMainPlayer::ToggleCrouch()
{
	if (IsBuildingInputBlocked()) return;

	//기절 중이면 못함
	if (IsInability) return;
	
	//웅크리는 중이면
	if (IsCrouching)
	{
		UnCrouch();
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		//GetCapsuleComponent()->SetCapsuleHalfHeight(DefaultHeight);
		IsCrouching = false;
	} else
	{
		if (GetCharacterMovement()->IsFalling()) return;
		
		Crouch();
		GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
		//GetCapsuleComponent()->SetCapsuleHalfHeight(CrouchHeight);
		IsCrouching = true;
	}
}

void AMainPlayer::ToggleInventory()
{
	if (IsBuildingInputBlocked()) return;

	
	if (IsLocallyControlled() && InventoryWidgetClass)
	{
		InventoryWidget = CreateWidget<UInventory>(GetWorld(), InventoryWidgetClass);
		FInputModeUIOnly InputMode;
		GetLocalViewingPlayerController()->SetInputMode(InputMode);
		GetLocalViewingPlayerController()->bShowMouseCursor = true;
		InventoryWidget->AddToViewport();
		//InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	return;
	
	//인벤토리가 열려 있으면
	if (IsInventoryOpen)
	{
		if (InventoryWidget)
		{
			if (HUD) HUD->SetVisibility(ESlateVisibility::Visible);
			InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
			
			FInputModeGameOnly InputMode;
			GetLocalViewingPlayerController()->SetInputMode(InputMode);
			GetLocalViewingPlayerController()->bShowMouseCursor = false;
			IsInventoryOpen = false;
		}
		
	}
	//인벤토리가 닫혀 있으면
	else
	{
		
		if (InventoryWidget)
		{
			if (HUD) HUD->SetVisibility(ESlateVisibility::Collapsed);
			InventoryWidget->SetVisibility(ESlateVisibility::Visible);
			
			FInputModeUIOnly InputMode;
			GetLocalViewingPlayerController()->SetInputMode(InputMode);
			GetLocalViewingPlayerController()->bShowMouseCursor = true;
			IsInventoryOpen = true;
			
			InventoryWidget->SetFocus();
			InventoryWidget->SetKeyboardFocus();
		}
		
		
	}
}

void AMainPlayer::SetBuildingInputBlocked(bool bBlocked)
{
	bBuildingInputBlocked = bBlocked;

	if (bBuildingInputBlocked && IsLocallyControlled())
	{
		GetCharacterMovement()->StopMovementImmediately();
	}
}

//TODO: 아이템 사용 로직 멀티로 전환하기
void AMainPlayer::UseItem(int32 SlotIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("USE ITEM EXECUTED"));
	
	if (InventoryComponent)
	{
		FItemData* ItemData = InventoryComponent->GetItemDataAtIndex(SlotIndex);
		
		if (ItemData)
		{
			if (ItemData->IsNotEmpty())
			{
				UE_LOG(LogTemp, Warning, TEXT("TRY TO USE THIS : [ %s ]"), *ItemData->TextData.Name.ToString());

			
				if (ItemData->ID == FName("FO019"))
				{
					if (StatusComponent)
					{
						if (EatingSound)
						{
							Multi_PlaySoundAtLocation(EatingSound, GetActorLocation());
						}
						StatusComponent->ApplyItem(*ItemData);
					}

					FItemBaseData SlotItem = InventoryComponent->GetItemAtIndex(SlotIndex);
				
					SlotItem.CurrentDurability -= 1.0f;

					if (SlotItem.CurrentDurability <= 0.0f)
					{
						InventoryComponent->Request_RemoveItemAmountAtSlot(SlotIndex, 1);
					
						FItemBaseData RewardItem = InventoryComponent->CreateItemByID(FName("FQ005"), 1);
						if (RewardItem.IsValid())
						{
							InventoryComponent->Request_HandleAddItem(RewardItem);
						}
					}
					else
					{
						InventoryComponent->Request_SetItemAtSlot(SlotIndex, SlotItem);
					}
				
					return; 
				}
			
			
				if (StatusComponent && ItemData->Type == EItemType::FOOD)
				{
					if (ItemData->Type == EItemType::FOOD && EatingSound)
					{
						Multi_PlaySoundAtLocation(EatingSound, GetActorLocation());
					}
					StatusComponent->ApplyItem(*ItemData);
					//TODO: 서버 호출 함수로 변경
					InventoryComponent->Request_RemoveItemAmountAtSlot(SlotIndex, 1);
				}
			} else
			{
				UE_LOG(LogTemp, Warning, TEXT("NO ITEM IN HOTBAR SLOT"));
			}
		}
	}
}

//타이머 시간을 0으로 하면 실행이 안되는 사실 발견...
void AMainPlayer::StartUseItem()
{
	if (!IsUsingItem)
	{
		IsUsingItem = true;
		
		FItemBaseData TargetItem = InventoryComponent->GetItemAtIndex(HotBarIndex);
		int32 TargetIndex = HotBarIndex;
		if (!TargetItem.IsValid()) return;
	
		//사용 가능한 아이템이 아니면 암것두 안하긔.
		if (!InventoryComponent->IsUsableItem(TargetItem)) return;
	
		if (!GetWorld()->GetTimerManager().IsTimerActive(ItemUseTimer))
		{
			FItemData* ItemData = InventoryComponent->GetItemData(TargetItem);
			
			if (!ItemData) return;

			//사용까지 꾹 눌러야 하는 시간
			float UseDuration = ItemData->NumericData.UseDuration;
			
			//음식이면 먹는 소리 재생
			if (TargetItem.Type == EItemType::FOOD && DuringFoodSound)
			{
				Multi_PlaySoundAtLocation(DuringFoodSound, GetActorLocation());
			}
		
			GetWorld()->GetTimerManager().SetTimer(
			ItemUseTimer,
			[this, TargetIndex]()
			{
				UseItem(HotBarIndex);
			},
			UseDuration,
			false
			);
		}
	}
}

void AMainPlayer::StopUseItem()
{
	if (IsUsingItem)
	{
		GetWorld()->GetTimerManager().ClearTimer(ItemUseTimer);
		IsUsingItem = false;
	}
}

void AMainPlayer::HandleHotBar(const FInputActionValue& Value)
{
	if (IsBuildingInputBlocked()) return;

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

		if (Subsystem)
		{
			for (FKey Key : Subsystem->QueryKeysMappedToAction(HotBarAction))
			{
				if (PlayerController->IsInputKeyDown(Key))
				{
					if (Key == EKeys::One)      Request_SetHotbarIndex(0);
					else if (Key == EKeys::Two) Request_SetHotbarIndex(1);
					else if (Key == EKeys::Three) Request_SetHotbarIndex(2);
					else if (Key == EKeys::Four) Request_SetHotbarIndex(3);
					else if (Key == EKeys::Five) Request_SetHotbarIndex(4);
					else if (Key == EKeys::Six)  Request_SetHotbarIndex(5);
				}
			}
		}
	}
}

void AMainPlayer::HandleHotBarWithWheel(const FInputActionValue& Value)
{
	if (IsBuildingInputBlocked()) return;

	if (UBuildingComponent* BuildComp = FindComponentByClass<UBuildingComponent>())
	{
		if (BuildComp->GetCurrentState() == EBuildingState::Placing)
		{
			BuildComp->RotatePreview(Value.Get<float>());
			return;
		}
	}

	if (Value.Get<float>() > 0)
	{
		Request_SetHotbarIndex((HotBarIndex + 1) % 6);
	} else
	{
		Request_SetHotbarIndex((HotBarIndex - 1 + 6) % 6);
	}
}

void AMainPlayer::SetHotbarIndex(int32 Index)
{
	HotBarIndex = Index;
	FItemBaseData Item = InventoryComponent->GetItemAtIndex(HotBarIndex);
	WeaponComponent->CheckWeapon(Item);
}

void AMainPlayer::OnDeath_Implementation()
{	
	//멀티 플레이 죽음 시
	//1. 10초간 기절 : 다른 플레이어가 붕대로 상호작용 시 회복
	//2. 10초 뒤 사망 후 리스폰 지역에서 부활
	if (GetWorld()->GetGameState<AMainGameState>()->IsMulti)
	{
		UE_LOG(LogTemp, Display, TEXT("[MULTI]Player Dead"));
		KnockOut();
	}
	//싱글 플레이 죽음 시 - 사망한 당일 아침으로 부활
	else
	{
		UE_LOG(LogTemp, Display, TEXT("[SINGLE]Player Dead"));
		Client_ShowDeathScreen();
	}
}

//손에 든 아이템 업데이트 함수
void AMainPlayer::RefreshHand()
{
	UE_LOG(LogTemp, Warning, TEXT("[PLAYER] Refresh Hand"));
	
	if (!InventoryComponent || !WeaponComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PLAYER] Inven & Weapon Comp is not Ready in REFRESH HAND"))
		return;
	}
	
	//핫바 인덱스의 아이템 정보 가져오기.
	FItemBaseData Item = InventoryComponent->GetItemAtIndex(HotBarIndex);
	
	//해당 인덱스 인벤토리 칸에 아이템이 있으면 그 아이템 들기.
	if (Item.IsValid())
	{	
		//데이터 베이스에서 아이템 데이터 가져오기
		FItemData* ItemData = InventoryComponent->GetItemData(Item);
		
		if (ItemData)
		{
			if (ItemData->Type == EItemType::EQUIPMENT || ItemData->Type == EItemType::FOOD)
			{
				IsHoldingItem = true;
			
				//토치 들기
				if (ItemData->ID == TEXT("EQ006"))
				{
					ItemMesh->SetStaticMesh(nullptr);
					WeaponComponent->UnequipeWeapon();

					// 토치 전용 애님 레이어 적용 (무기 DT 독립)
					if (TorchAnimLayerClass)
					{
						GetMesh()->LinkAnimClassLayers(TorchAnimLayerClass);
					}
				
					if (Torch)
					{
						Torch->SetActorHiddenInGame(false);
					}
				
					return;
				}
			
				ItemMesh->SetStaticMesh(ItemData->AssetData.Mesh);
				ItemMesh->AttachToComponent(
				GetMesh(),
				FAttachmentTransformRules::KeepRelativeTransform,
				TEXT("hand_r"));
				// 메시 에셋의 콜리전 응답을 완전히 초기화 후 Weapon 트레이스 채널만 허용
				ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
				ItemMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECollisionResponse::ECR_Block); // Weapon 채널
			
				FTransform SocketTransform = ItemMesh->GetSocketTransform(TEXT("HandSocket"), RTS_Component);
				ItemMesh->SetRelativeTransform(SocketTransform.Inverse());
				WeaponComponent->CheckWeapon(Item);
			} 
			else
			{
				WeaponComponent->CheckWeapon(Item);
				IsHoldingItem = false;
				ItemMesh->SetStaticMesh(nullptr);
			}
		}
		else
		{
			WeaponComponent->CheckWeapon(Item);
			IsHoldingItem = false;
			ItemMesh->SetStaticMesh(nullptr);
		}
	}
	else
	{
		WeaponComponent->CheckWeapon(Item);
		IsHoldingItem = false;
		ItemMesh->SetStaticMesh(nullptr);
	}
	
	if (Torch)
	{
		Torch->SetActorHiddenInGame(true);
	}
}

void AMainPlayer::Attack()
{
	if (IsSwimming || IsInability) return;
	
	if (UBuildingComponent* BuildComp = FindComponentByClass<UBuildingComponent>())
	{
		/*int32 StateInt = (int32)BuildComp->GetCurrentState();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, 
			FString::Printf(TEXT("클라이언트 Attack 호출됨! 현재 상태: %d"), StateInt));*/

		if (BuildComp->GetCurrentState() == EBuildingState::Placing)
		{
			BuildComp->ConfirmBuild(); 
			return; 
		}
	}
	
	if (WeaponComponent) {
		WeaponComponent->Request_UseWeapon();
	}
}

void AMainPlayer::KnockOut()
{
	UE_LOG(LogTemp, Display, TEXT("[%hs][PLAYER] KNOCKOUT"), HasAuthority()?"SERVER":"CLIENT");
	//일단 기본 스탠드 상태로 전환
	if (IsCrouching) Request_ToggleCrouch();
	if (IsRunning) Request_StopRun();
	
	Request_StopEmotion();
	
	//기절 타이머 실행 - 누가 소생시켜주지 않으면 10초 뒤 사망
	GetWorld()->GetTimerManager().SetTimer(
		KnockOutTimer,
		[this]()
		{
			UE_LOG(LogTemp, Warning, TEXT("[PLAYER] KNOCK OUT TIMER ACTIVATED"));
			if (HasAuthority())
			{
				UE_LOG(LogTemp, Warning, TEXT("[PLAYER] SERVER PLAYER"));
				//서버 캐릭터면 사망 카메라 세팅
				if (IsLocallyControlled())
				{
					FirstPersonCamera->PostProcessSettings.bOverride_ColorSaturation = true;
					FirstPersonCamera->PostProcessSettings.ColorSaturation = FVector4(0,0,0,0);
		
					if (InventoryWidget) InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
					
					if (AMainPlayerController* PC = GetController<AMainPlayerController>())
					{
						PC->OpenDeathScreen();
					}
				}
				//클라 캐릭터면 사망 카메라 클라 함수 실행
				else
				{
					Client_ShowDeathScreen();
				}
			}
		},
		KnockOutToDeathTime,
		false);
	
	//기절 상태로 전환
	IsInability = true;
	GetCharacterMovement()->MaxWalkSpeed = KnockOutSpeed;
	CanInteract = true;
	InteractableData.CanInteract = CanInteract;
}

void AMainPlayer::Revive()
{
	GetWorld()->GetTimerManager().ClearTimer(KnockOutTimer);
	StatusComponent->IncreaseHP(20.0f);
	IsInability = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	CanInteract = false;
	InteractableData.CanInteract = CanInteract;

	// 만약 변신했다가 기절해서 돌아온 상태라면, 세션 데이터 삭제 필요
	if (HasAuthority())
	{
		AMoonlightInfectionSystem* System = Cast<AMoonlightInfectionSystem>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AMoonlightInfectionSystem::StaticClass()));
		if (System && GetController())
		{
			// 부활했으므로 늑대인간 세션 아님
			System->ActiveWerewolfSessions.Remove(Cast<APlayerController>(GetController()));
		}
	}
}

void AMainPlayer::OnRespawn()
{
	StatusComponent->IncreaseHP(20.0f);
	
	//안 기절 상태로 전환
	IsInability = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	CanInteract = false;
	InteractableData.CanInteract = CanInteract;
		
	RestoreCamera();
}

void AMainPlayer::RestoreCamera_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("[PLAYER] RESTORE CAMERA"));
	FirstPersonCamera->PostProcessSettings.bOverride_ColorSaturation = true;
	FirstPersonCamera->PostProcessSettings.ColorSaturation = FVector4(1, 1,1,1);
}

void AMainPlayer::CheckInteraction()
{
	//플레이어 시야 카메라 체크
	if (FirstPersonCamera)
	{
		InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();
		//트레이스 시작 지점
		FVector TraceStart{ FirstPersonCamera->GetComponentLocation() };
		//트레이스 종료 지점
		FVector TraceEnd{ TraceStart + (FirstPersonCamera->GetForwardVector() * InteractionCheckDistance) };

		//라인 트레이스 디버그 라인
		//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 1.0f);
		
		//자기 메쉬에 안부딪히게 설정
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		//충돌 결과 변수
		FHitResult HitResult;

		//라인트레이스 실행 후 부딪혔나?
		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_GameTraceChannel6, QueryParams))
		{
			/*
			//타겟 HP 보여줄 지 안보여줄 지
			if (UStatusComponent* Status = HitResult.GetActor()->GetComponentByClass<UStatusComponent>())
			{
				//UE_LOG(LogTemp, Warning, TEXT("[PLAYER] HIT ACTOR HAS STATUS COMPONENT"));
				if (Status->ShowCurrentHP)
				{
					HUD->DisplayTargetHP(HitResult.GetActor());
				} else
				{
					//UE_LOG(LogTemp, Warning, TEXT("[PLAYER] ShowCurrentHP is FALSE"));
				}
			} else
			{
				//UE_LOG(LogTemp, Warning, TEXT("[PLAYER] HIT ACTOR HAS NO STATUS COMPONENT"));
			}
			*/
			
			//부딪힌 액터가 인터랙션 인터페이스를 가지고 있나?
			if (HitResult.GetActor() && HitResult.GetActor()->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
			{
				//부딪힌 액터가 현재 인터랙터블 데이터와 다르다면
				if (HitResult.GetActor() != InteractionData.CurrentInteractable)
				{
					//UE_LOG(LogTemp, Warning, TEXT("FoundInteractable"));
					//TargetInteractable에 결과물 넣기
					UE_LOG(LogTemp, Warning, TEXT("[PLAYER] INTERACTABLE : %s"), *HitResult.GetActor()->GetName())
					FoundInteractable(HitResult.GetActor());
					return;
				}

				//부딪힌 액터가 현재 인터랙터블 액터와 같다면 암것두 안하기~
				if (HitResult.GetActor() == InteractionData.CurrentInteractable)
				{
					return;
				}
			}
			// 폴리지 체크
			else if (UInstancedStaticMeshComponent* ISMC = Cast<UInstancedStaticMeshComponent>(HitResult.GetComponent()))
			{
				UStaticMesh* HitMesh = ISMC->GetStaticMesh();
				// 통합된 Map에 등록된 폴리지인지 확인
				if (FoliageRewardMap.Contains(HitMesh))
				{
					int32 InstanceIndex = HitResult.Item;
					if (InteractionData.CurrentFoliageComponent != ISMC || InteractionData.FoliageInstanceIndex != InstanceIndex)
					{
						FoundInteractableFoliage(ISMC, InstanceIndex);
					}
					return;
				}
			}
			// 물 체크
			AActor* HitActor = HitResult.GetActor();
			UPrimitiveComponent* HitComp = HitResult.GetComponent();
			
			if (HitActor)
			{
				FString ClassName = HitActor->GetClass()->GetName();
				if (ClassName.Contains(TEXT("WaterBodyOcean")) || 
					ClassName.Contains(TEXT("WaterBodyRiver")) || 
					ClassName.Contains(TEXT("WaterBodyLake")))
				{
					//UE_LOG(LogTemp, Warning, TEXT("[PLAYER] FIND WATER : %s"), *ClassName);
					if (GetWorld()->GetTimeSeconds() >= LastDrinkTime + DrinkCooldown)
					{
						if (InteractionData.CurrentWaterComponent != HitComp)
						{
							FoundInteractableWater(HitComp);
						}
						return;
					}
					return;
				}
			}
		}
		/*
		else
		{
			HUD->HideTargetHP();
		}
		*/
	}
	NotFoundInteractable();
}

void AMainPlayer::FoundInteractable(AActor* Interactable)
{
	//UE_LOG(LogTemp, Warning, TEXT("[PLAYER] FOUND IMTERACTABLE"));
	
	if (IsInteracting()) 
	{
		EndInteract();
	}
	
	NotFoundInteractable();

	// 새로운 인터렉션 대상 지정
	InteractionData.CurrentInteractable = Interactable;
	TargetInteractionInterface = Interactable;
	
	if (!IsValid(TargetInteractionInterface.GetObject())) 
	{
		return;
	}
	
	//인터랙터블 액터의 상태가 인터랙션 가능한 상태가 아니면
	if (!TargetInteractionInterface->InteractableData.CanInteract)
	{
		//UE_LOG(LogTemp, Warning, TEXT("[PLAYER] THIS ACTOR CAN'T INTERACT"));
		if (IsLocallyControlled())
		{
			if (IsValid(HUD)) HUD->DisplayDefault();
		} else
		{
			//UE_LOG(LogTemp, Warning, TEXT("[PLAYER] NOT LOCALLY CONTROLLED : Can't Change Aim to Unfocus"));
		}
		TargetInteractionInterface->Execute_EndFocus(InteractionData.CurrentInteractable);
		return;
	} else
	{
		//UE_LOG(LogTemp, Warning, TEXT("[PLAYER] THIS ACTOR CAN INTERACT"));
	}
	if (IsLocallyControlled())
	{
		if (IsValid(HUD)) HUD->DisplayInteractable();
	} else
	{
		//UE_LOG(LogTemp, Warning, TEXT("[PLAYER] NOT LOCALLY CONTROLLED : Can't Change Aim to Focus"));
		
	}
	TargetInteractionInterface->Execute_BeginFocus(InteractionData.CurrentInteractable);
}

//인터랙션 가능 액터를 못찾았을 때
void AMainPlayer::NotFoundInteractable()
{
	// 인터랙션 중이면 타이머 해제
	if (IsInteracting())
	{
		GetWorldTimerManager().ClearTimer(InteractionTimer);
	}

	// 1. 기존에 바라보던 액터가 있었다면 포커스 해제
	if (IsValid(InteractionData.CurrentInteractable)) 
	{		
		if (AActor* Target = Cast<AActor>(TargetInteractionInterface.GetObject()))
		{
			Request_EndInteractPlayer(Target);
		}
		
		if (IsValid(TargetInteractionInterface.GetObject()))
		{
			TargetInteractionInterface->Execute_EndFocus(InteractionData.CurrentInteractable);			
		}

		if (IsLocallyControlled())
		{
			if (IsValid(HUD))
			{
				HUD->DisplayDefault();
				HUD->HideInteraction();
			}
		}
		
		InteractionData.CurrentInteractable = nullptr;
		TargetInteractionInterface = nullptr;
	}

	// 2. 아웃라인 액터 파괴 및 폴리지 데이터 초기화
	if (IsValid(CurrentOutlineActor))
	{
		CurrentOutlineActor->Destroy();
	}
	CurrentOutlineActor = nullptr;

	// 3. 폴리지 및 물 데이터 무조건 초기화
	InteractionData.CurrentFoliageComponent = nullptr;
	InteractionData.FoliageInstanceIndex = INDEX_NONE;
	InteractionData.CurrentWaterComponent = nullptr;
}

void AMainPlayer::Client_InteractionExecuted_Implementation()
{
	if (HUD)
	{
		HUD->HideInteraction();
	}
}

//인터랙션 시작 함수 (인터랙션 키 눌렀을 때)
void AMainPlayer::BeginInteract()
{
	UE_LOG(LogTemp, Warning, TEXT("[PLAYER] BeginInteract"));
	if (IsBuildingInputBlocked()) return;

	//인터랙션이 시작됐을 때부터 인터렉션 상태가 변하지 않는 것을 체크
	CheckInteraction();

	if (InteractionData.CurrentInteractable && IsValid(TargetInteractionInterface.GetObject()))
	{
		//인터랙션 데이터가 있으면
		if (InteractionData.CurrentInteractable)
		{
			//인터랙션 타겟 액터
			AActor* Target = Cast<AActor>(TargetInteractionInterface.GetObject());
			
			//타겟이 플레이어일 때, 그 플레이어에게 내가 너 인터랙팅 중임! 이라고 알림
			Request_BeginInteractPlayer(Target);
			
			//인터랙션 액터의 인터랙션 시작 함수 실행
			//TargetInteractionInterface->BeginInteract();
			//즉시 인터랙션이 가능하면 (꾹 누르는 인터랙션이 아니면)
			if (TargetInteractionInterface->InteractableData.InteractionDuration == 0.0f)
			{
				//인터랙션 가능 상태인지 확인
				if (TargetInteractionInterface->InteractableData.CanInteract)
				{
					//인터랙션 실행
					Interaction(Target);
				}
			}
			//꾹 누르는 인터랙션이면
			else
			{
				//인터랙션 가능 상태인지 확인
				if (TargetInteractionInterface->InteractableData.CanInteract)
				{
					if (HUD) HUD->DisplayInteraction();
					
				// 기절한 플레이어인 경우 치유 몽타주 재생
				if (AMainPlayer* TargetPlayer = Cast<AMainPlayer>(Target))
				{
					if (TargetPlayer->IsInability && HealMontage)
					{
						Server_PlayHealMontage();
					}
				}
					
					//인터랙션 실행 시간 만큼 대기 후 인터랙션 실행
					GetWorldTimerManager().SetTimer(InteractionTimer,
						[this, Target]()
						{
							//인터랙션 실행
							Interaction(Target);
						},
						TargetInteractionInterface->InteractableData.InteractionDuration,
						false);
				}
			}
		}
	}
	else if (InteractionData.CurrentFoliageComponent && InteractionData.FoliageInstanceIndex != INDEX_NONE)
	{
		Server_InteractFoliage(InteractionData.CurrentFoliageComponent, InteractionData.FoliageInstanceIndex);
	}
	else if (InteractionData.CurrentWaterComponent)
	{
		Server_DrinkWater(InteractionData.CurrentWaterComponent);
	}
}

void AMainPlayer::EndInteract()
{
	UE_LOG(LogTemp, Warning, TEXT("[PLAYER] EndInteract"));
	if (IsBuildingInputBlocked()) return;
	
	if (HUD)
	{
		HUD->HideInteraction();
	}
	
	if (InteractionData.CurrentInteractable)
	{
		//인터랙션 타겟 액터
		AActor* Target = Cast<AActor>(TargetInteractionInterface.GetObject());
		
		//인터랙션 타겟이 플레이어면 그만 만진다고 알림!
		Request_EndInteractPlayer(Target);
	}
	
	if (HealMontage)
	{
		Server_StopHealMontage();
	}
	
	//인터랙션 타이머 클리어
	GetWorldTimerManager().ClearTimer(InteractionTimer);

	//인터랙션 액터가 유효한 지 체크
	if (IsValid(TargetInteractionInterface.GetObject()))
	{
		//인터랙션 액터의 인터랙션 종료 함수 실행
		//TargetInteractionInterface->EndInteract();
	}
}

void AMainPlayer::Interact_Implementation(AActor* Interactor)
{
	UE_LOG(LogTemp, Warning, TEXT("[PLAYER] Interact Executed"));
	Revive();
}

void AMainPlayer::Interaction_Implementation(AActor* Target)
{
	UE_LOG(LogTemp, Warning, TEXT("[%hs] SERVER INTERACTION EXECUTED"), HasAuthority()?"SERVER":"CLIENT")
	//인터랙션 타이머 클리어
	GetWorldTimerManager().ClearTimer(InteractionTimer);
	if (IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PLAYER][SERVER] Interaction Complete. Hide Bar"));
		if (HUD) HUD->HideInteraction();
	} else
	{
		Client_InteractionExecuted();
	}
	
	//인터랙션 액터가 유효한 지 체크
	if (IsValid(Target))
	{
		TargetInteractionInterface = Target;	
		//인터랙션 액터가 인터랙션 가능한 상태이면
		if (TargetInteractionInterface->InteractableData.CanInteract)
		{
			//인터랙션 액터의 인터랙션 함수 실행
			TargetInteractionInterface->Execute_Interact(Target, this);
			
			Request_EndInteractPlayer(Target);
		}
	}
}

void AMainPlayer::DropItem(UInventoryComponent* SourceInventory, int32 SourceIndex, int32 AmountToDrop, bool IsWhole)
{
	FItemBaseData ItemData = SourceInventory->GetInventory()[SourceIndex].ItemData;
	
	//아이템 데이터가 있으면
	if (ItemData.IsValid() && ItemClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		const FVector SpawnLocation(GetActorLocation() + (GetActorForwardVector() * 50.0f));
		const FTransform SpawnTransform(GetActorRotation(), SpawnLocation);
		
		if (IsWhole)
		{
			SourceInventory->Request_RemoveItemAmountAtSlot(SourceIndex, AmountToDrop);
			SourceInventory->InventoryChanged();
		}
		
		APickup* Pickup = GetWorld()->SpawnActor<APickup>(ItemClass, SpawnTransform, SpawnParams);
		Pickup->InitializeDrop(ItemData, AmountToDrop);
		
		if (ItemGettingSound)
		{
			Client_PlaySound2D(ItemGettingSound);
		}
		
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("CAN'T FIND MATCHED ITEM."))
	}
}

FItemBaseData AMainPlayer::GetHoldingItemReference()
{
	if (InventoryComponent)
	{
		if (InventoryComponent->GetItemAmount() > 0)
		{
			return InventoryComponent->GetItemAtIndex(HotBarIndex);
		}
	}
	
	return FItemBaseData();
}

EItemType AMainPlayer::GetHoldingItemType()
{
	FItemBaseData HoldingItem = GetHoldingItemReference();
	
	if (HoldingItem.IsValid())
	{
		FItemData* ItemData = InventoryComponent->GetItemData(HoldingItem);
		
		return ItemData->Type;
	}
	
	return EItemType::MATERIAL;
}

void AMainPlayer::WeaponTrace(const FVector& StartPos, const FVector& EndPos)
{
	if (!HasAuthority()) return;

	//트레이스 파라미터 설정
	ETraceTypeQuery TraceTypeQuery = UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel4);
	//ECC_GameTraceChannel4 <- Weapon 채널
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);
	FHitResult Hit;
	AActor* HitActor = Hit.GetActor();

	//스피어 트레이스 실행
	if (UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		StartPos,
		EndPos,
		1.0f,
		TraceTypeQuery,
		true,
		IgnoreActors,
		//EDrawDebugTrace::None,
		EDrawDebugTrace::ForDuration,
		Hit,
		true))
	{
		// 기본 대미지
		float DamageAmount = 0.0f;
		FItemBaseData HoldingItem = GetHoldingItemReference();

		//무기 장착 시 무기 대미지로 설정
		if (HoldingItem.IsValid())
		{
			if (FItemData* ItemData = InventoryComponent->GetItemData(HoldingItem))
			{
				DamageAmount = ItemData->NumericData.Damage;
			}
		}

		//최초로 맞고 또 맞은 액터면 무시
		if (DamagedActors.Contains(HitActor)) return;

		//최초로 맞은 액터면 맞은 액터 배열에 추가
		DamagedActors.Add(HitActor);

		//판정 함수 호출 (맞은 정보와 대미지를 전달)
		ProcessAttackHit(Hit, DamageAmount);
	}
	
	FVector AimPos = FirstPersonCamera->GetComponentLocation();
	FVector AimStartPos = AimPos + FirstPersonCamera->GetForwardVector() * 10.0f;
	FVector AimEndPos = AimPos + FirstPersonCamera->GetForwardVector() * 100.0f;
	
	//에임에 트레이스 하나 더 실행
	if (UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		AimStartPos,
		AimEndPos,
		5.0f,
		TraceTypeQuery,
		true,
		IgnoreActors,
		//EDrawDebugTrace::None,
		EDrawDebugTrace::ForDuration,
		Hit,
		true))
	{
		// 기본 대미지
		float DamageAmount = 0.0f;
		FItemBaseData HoldingItem = GetHoldingItemReference();

		//무기 장착 시 무기 대미지로 설정
		if (HoldingItem.IsValid())
		{
			if (FItemData* ItemData = InventoryComponent->GetItemData(HoldingItem))
			{
				DamageAmount = ItemData->NumericData.Damage;
			}
		}

		//최초로 맞고 또 맞은 액터면 무시
		if (DamagedActors.Contains(HitActor)) return;

		//최초로 맞은 액터면 맞은 액터 배열에 추가
		DamagedActors.Add(HitActor);

		//판정 함수 호출 (맞은 정보와 대미지를 전달)
		ProcessAttackHit(Hit, DamageAmount);
	}
}

//공격 트레이스 시작 함수
void AMainPlayer::StartWeaponAttack()
{
	
	//클라이언트 호출이면 유기
	if (!HasAuthority()) return;
	
	//서버 실행일 때만 트레이스 시작
	IsTracingAttack = true;
	DamagedActors.Empty();
	//히트 포인트 목록 비우기
	TracePoints.Empty();
	
	for (FName Socket : HitSockets)
	{
		//무기가 있으면 무기에서 히트 포인트 찾기
		if (WeaponComponent->IsEquipped)
		{
			//있는 히트 포인트만 넣기
			if (ItemMesh->DoesSocketExist(Socket))
			{
				FVector Pos = ItemMesh->GetSocketLocation(Socket);
				TracePoints.Add(Socket, { ItemMesh, Pos, Pos });
			}
		}
		//맨손이면 손의 히트 포인트 찾기
		else
		{
			//있는 히트 포인트만 넣기
			if (GetMesh()->DoesSocketExist(Socket))
			{
				FVector Pos = GetMesh()->GetSocketLocation(Socket);
				TracePoints.Add(Socket, { GetMesh(), Pos, Pos });
			}
		}
	}
}

//공격 트레이스 종료 함수
void AMainPlayer::EndWeaponAttack()
{
	//클라이언트 호출이면 유기
	if (!HasAuthority()) return;
	
	//서버 실행일 때만 트레이스 종료
	//공격 트레이스 종료
	IsTracingAttack = false;
	//맞은 액터 배열 비우기
	DamagedActors.Empty();
	//히트 포인트 목록 비우기
	TracePoints.Empty();
}

void AMainPlayer::DropItemOnHotBar()
{
	if (IsBuildingInputBlocked()) return;

	FItemBaseData Item = InventoryComponent->GetItemAtIndex(HotBarIndex);
	if (Item.IsValid())
	{
		Request_DropItem(InventoryComponent, HotBarIndex, 1, true);
	}
}

void AMainPlayer::WaterElevation(const FInputActionValue& Value)
{
	if (IsBuildingInputBlocked()) return;

	if (IsSwimming)
	{
		float Elevation = Value.Get<float>();
		
		AddMovementInput(GetActorUpVector(), Elevation*WaterDeceleration);
	}
}

void AMainPlayer::EnterWater(const FSphericalPontoon& Pontoon)
{
	UE_LOG(LogTemp, Warning, TEXT("Entering Water"));
	IsSwimming = true;
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	StatusComponent->StartAir();
	EnteredWater = BuoyancyComponent->GetCurrentWaterBodyComponents()[0];
	SetSwimMode(ESwimMode::TREADING);
	
	GetWorld()->GetTimerManager().SetTimer(
		SwimCheckHandle,
		this,
		&AMainPlayer::SwimCheck,
		0.01f,
		true);
}

void AMainPlayer::ExitWater(const FSphericalPontoon& Pontoon)
{
	UE_LOG(LogTemp, Warning, TEXT("Exiting Water"));
	IsSwimming = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	EnteredWater = nullptr;
	SetSwimMode(ESwimMode::NONE);
	GetWorld()->GetTimerManager().ClearTimer(SwimCheckHandle);
}

void AMainPlayer::HideAirBar()
{
	if (IsLocallyControlled()&&HUD)
	{
		HUD->HideAirBar();
	}
}

void AMainPlayer::SwimCheck()
{
	//올라갈 때 수면 위로 너무 올라가지 않게 하고, 발에 땅이 닿는 수위면 수영 종료로 바뀌게 체크하는 함수...
	//들어간 물을 감지할 수 없으면 탈출
	if (!EnteredWater) return;
	
	//현 위치의 수면위치, 수면 노멀, 유속, 수심 데이터
	FVector SurfaceLocation;
	FVector SurfaceNormal;
	FVector WaterVelocity;
	float WaterDepth;
	
	//플레이어 위치
	FVector OriginLocation = GetActorLocation();
	
	//구하기
	EnteredWater->GetWaterSurfaceInfoAtLocation(
		OriginLocation, 
		SurfaceLocation, 
		SurfaceNormal, 
		WaterVelocity,
		WaterDepth,
		true);
	
	FWaveInfo WaveInfo;
	
	EnteredWater->GetWaveInfoAtPosition(SurfaceLocation, WaterDepth, false, WaveInfo);
	
	//디버그 출력
	//UE_LOG(LogTemp, Warning, TEXT("Surface Z : %f"), SurfaceLocation.Z);
	//UE_LOG(LogTemp, Warning, TEXT("WAVE Z : %f"), WaveInfo.Height);
	
	//플레이어 발 위치
	FVector FootCheckLocation = FVector(
		OriginLocation.X,
		OriginLocation.Y,
		OriginLocation.Z-90.0f);
	//트레이스에 무시할 액터들
	TArray<AActor*> Ignores;
	Ignores.Add(this);
	//충돌 결과
	FHitResult Hit;
	
	//발이 닿으면 수면 탈출 가능
	bool CanStand = UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		FootCheckLocation,
		FootCheckLocation,
		25.0f,
		UEngineTypes::ConvertToTraceType(ECC_WorldStatic),
		false,
		Ignores,
		EDrawDebugTrace::None,
		Hit,
		true);
	
	//수면 위로 못나가게 위치 조정
	if (!CanStand && GetActorLocation().Z >= SurfaceLocation.Z-WaterSurfaceOffset)
	{
		FVector TargetLocation = FVector(
			OriginLocation.X, 
			OriginLocation.Y, 
			SurfaceLocation.Z-WaterSurfaceOffset);
		
		FVector AdjustedLocation = UKismetMathLibrary::VInterpTo(OriginLocation, TargetLocation, GetWorld()->GetDeltaSeconds(), 10.0f);
		SetActorLocation(AdjustedLocation);
	}
	
	//수심에 따른 수영 모드 변환
	if (SwimMode!=ESwimMode::UNDERWATER_IDLE && OriginLocation.Z <= SurfaceLocation.Z-WaterSuffocatedOffest)
	{
		SetSwimMode(ESwimMode::UNDERWATER_IDLE);
	}
	
	if (SwimMode!=ESwimMode::TREADING && OriginLocation.Z >= SurfaceLocation.Z-WaterSurfaceOffset)
	{
		SetSwimMode(ESwimMode::TREADING);
	}
}

void AMainPlayer::SetSwimMode(ESwimMode NewSwimMode)
{
	SwimMode = NewSwimMode;
	
	//수영 모드에 따른 동작
	switch (SwimMode)
	{
		//수영 모드 아닐 때
	case ESwimMode::NONE:
		{
			StatusComponent->StopAir();
			StatusComponent->StopAirDeath();
			StatusComponent->StartRecoverAir();
			if (IsLocallyControlled())
			{
				WaterAmbience->FadeOut(1.0f,0);
			}
			break;
		}
		//수면에 머리 빼꼼
	case ESwimMode::TREADING:
		{
			StatusComponent->StopAir();
			StatusComponent->StopAirDeath();
			StatusComponent->StartRecoverAir();
			if (IsLocallyControlled())
			{
				WaterAmbience->FadeOut(0.5f,0);
			}
			break;
		}
		//수면 수영
	case ESwimMode::SURFACE_SWIMMING:
		{
			StatusComponent->StartAir();
			if (IsLocallyControlled())
			{
				WaterAmbience->FadeOut(0.5f,0);
			}
			break;
		}
		//수중 대기
	case ESwimMode::UNDERWATER_IDLE:
		{
			StatusComponent->StartAir();
			if (IsLocallyControlled())
			{
				if (HUD) HUD->DisplayAirBar();
				if (UnderWaterAmbience)
				{
					WaterAmbience->SetSound(UnderWaterAmbience);
					WaterAmbience->FadeIn(1.0f,0.2f);
				}
			}
			break;
		}
		//수중 수영
	case ESwimMode::UNDERWATER_SWIMMING:
		{
			StatusComponent->StartAir();
			if (IsLocallyControlled())
			{
				if (HUD) HUD->DisplayAirBar();
				if (UnderWaterAmbience)
				{
					WaterAmbience->SetSound(UnderWaterAmbience);
					WaterAmbience->FadeIn(1.0f,0.2f);
				}
			}
			break;
		}
	}
}

void AMainPlayer::Server_InteractFoliage_Implementation(UInstancedStaticMeshComponent* ISMC, int32 InstanceIndex)
{
	if (!HasAuthority()) return;
	
	if (!ISMC || InstanceIndex == INDEX_NONE) return;

	UStaticMesh* HitMesh = ISMC->GetStaticMesh();
	if (!HitMesh || !FoliageRewardMap.Contains(HitMesh)) return;

	FFoliageReward Reward = FoliageRewardMap[HitMesh];

	FTransform InstanceTransform;
	ISMC->GetInstanceTransform(InstanceIndex, InstanceTransform, true);

	if (InventoryComponent && !Reward.ItemID.IsNone() && Reward.ItemAmount > 0)
	{
		FItemBaseData NewItem = InventoryComponent->CreateItemByID(Reward.ItemID, Reward.ItemAmount);
		
		if (NewItem.IsValid())
		{
			FItemAddResult AddResult = InventoryComponent->HandleAddItem(NewItem);
			if (AddResult.OperationResult == EItemAddedResult::NoItemAdded)
			{
				InventoryComponent->Client_AddResult(AddResult);
				
				return; 
			}
			
			InventoryComponent->InventoryChanged();
			InventoryComponent->Client_AddResult(AddResult);
		}
	}

	if (Reward.SpawnBP)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(Reward.SpawnBP, InstanceTransform, SpawnParams);

		if (ARespawnableFoliage* RespawnableFoliage = Cast<ARespawnableFoliage>(SpawnedActor))
		{
			RespawnableFoliage->InitializeFoliage(ISMC, InstanceTransform);
		}
	}

	if (AMainGameMode* GM = Cast<AMainGameMode>(GetWorld()->GetAuthGameMode()))
	{
		FRemovedFoliageData RemovedData;
		RemovedData.Location = InstanceTransform.GetLocation();
		RemovedData.Rotation = InstanceTransform.GetRotation().Rotator();
		RemovedData.Scale = InstanceTransform.GetScale3D();
		RemovedData.Mesh = HitMesh;
		
		GM->RemovedFoliageData.Add(RemovedData);
		UE_LOG(LogTemp, Warning, TEXT("[PLAYER] Pebble Interact : Save Modified"))
		
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PLAYER] Pebble Interact : No GameMode"))
	}

	Multi_RemoveFoliageInstance(ISMC, InstanceIndex);

	if (CurrentOutlineActor)
	{
		CurrentOutlineActor->Destroy();
		CurrentOutlineActor = nullptr;
	}
}

void AMainPlayer::FoundInteractableFoliage(UInstancedStaticMeshComponent* ISMC, int32 InstanceIndex)
{
	if (InteractionData.CurrentInteractable || InteractionData.CurrentFoliageComponent || InteractionData.CurrentWaterComponent)
	{
		NotFoundInteractable(); 
	}

	// 폴리지 데이터 저장
	InteractionData.CurrentFoliageComponent = ISMC;
	InteractionData.FoliageInstanceIndex = InstanceIndex;

	// 외곽선용 BP 스폰 로직
	if (OutlineActorClass && ISMC && InstanceIndex != INDEX_NONE)
	{
		FTransform InstanceTransform;
		ISMC->GetInstanceTransform(InstanceIndex, InstanceTransform, true);
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CurrentOutlineActor = GetWorld()->SpawnActor<AActor>(OutlineActorClass, InstanceTransform, SpawnParams);

		if (CurrentOutlineActor)
		{
			UStaticMeshComponent* DummyMeshComp = CurrentOutlineActor->FindComponentByClass<UStaticMeshComponent>();
			if (DummyMeshComp)
			{
				DummyMeshComp->SetStaticMesh(ISMC->GetStaticMesh());
				
				DummyMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				DummyMeshComp->SetCastShadow(false);
			}
		}
	}
	
}

void AMainPlayer::TryConvertFoliageToActor(const FHitResult& HitResult, float DamageAmount)
{
	if (!HasAuthority()) return;

	// 1. 맞은 컴포넌트가 유효한지 먼저 확인
	UPrimitiveComponent* HitComponent = HitResult.GetComponent();
	if (!HitComponent) return;

	// 2. 폴리지(InstancedStaticMeshComponent)인지 변환 시도
	UInstancedStaticMeshComponent* ISMC = Cast<UInstancedStaticMeshComponent>(HitComponent);

	// 폴리지가 아니면 종료
	if (!ISMC) 
	{
		//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("맞은 건 폴리지가 아님"));
		return; 
	}

	// 3. 어떤 나무(StaticMesh)인지 확인
	UStaticMesh* HitMesh = ISMC->GetStaticMesh();

	// 4. 맵(목록)에 등록된 나무인지 확인
	if (!HitMesh || !FoliageToActorMap.Contains(HitMesh)) 
	{
		UE_LOG(LogTemp, Warning, TEXT("[PLAYER] THIS TREE IS NOT ON TREE MAP"))
		return;
	}

	// 5. 인덱스 확인 (가끔 -1이 들어오는 경우 방지)
	int32 InstanceIndex = HitResult.Item;
	if (InstanceIndex == INDEX_NONE) return;

	TSubclassOf<ATree> TargetActorClass = FoliageToActorMap[HitMesh];
	if (!TargetActorClass) return;

	FTransform InstanceTransform;
	// 월드 좌표 기준으로 트랜스폼 가져오기
	ISMC->GetInstanceTransform(InstanceIndex, InstanceTransform, true);
	
	//====>> 2.19 조성윤 추가 <<====Start
	//삭제될 폴리지 정보 저장
	//이 함수는 서버에서만 실행되니까 GetAuthGameMode이 null이 아님.
	AMainGameMode* GM = Cast<AMainGameMode>(GetWorld()->GetAuthGameMode());
	
	FRemovedFoliageData RemovedData;
	RemovedData.Location = InstanceTransform.GetLocation();
	RemovedData.Rotation = InstanceTransform.GetRotation().Rotator();
	RemovedData.Scale = InstanceTransform.GetScale3D();
	RemovedData.Mesh = ISMC->GetStaticMesh();
	
	GM->RemovedFoliageData.Add(RemovedData);
	//====>> 2.19 조성윤 추가 <<====End
	
	// 폴리지 삭제 (서버에서 삭제하면 리플리케이션 설정에 따라 클라이언트에게 전달됩니다)
	Multi_RemoveFoliageInstance(ISMC, InstanceIndex);

	// 진짜 액터 소환 (서버에서 스폰하면 모든 클라이언트에게 복제됩니다)
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATree* NewTree = GetWorld()->SpawnActor<ATree>(TargetActorClass, InstanceTransform, SpawnParams);
	NewTree->SetTreeMesh(HitMesh);
	NewTree->EnsureGUID();
	
	UE_LOG(LogTemp, Warning, TEXT("Spawn Tree class : %s"), *NewTree->GetClass()->GetName());
	// 데미지 전달
	if (NewTree)
	{
		if (!DamagedActors.Contains(NewTree))
		{
			DamagedActors.Add(NewTree);
		}
		
		float FinalDamage = 0.0f;
		FItemBaseData HoldingItem = GetHoldingItemReference();

		FinalDamage = UStatusComponent::CalculateFinalDamage(this, NewTree, DamageAmount);
		
		if (HoldingItem.IsValid())
		{
			if (FItemData* ItemData = InventoryComponent->GetItemData(HoldingItem))
			{
				if (ItemData->ID == FName(TEXT("EQ004")))
				{
					FinalDamage = UStatusComponent::CalculateTrueDamage(this, NewTree, DamageAmount);
					UE_LOG(LogTemp, Warning, TEXT("TrueDamage"));
				}
			}
		}
		
		UGameplayStatics::ApplyDamage(
			NewTree, 
			FinalDamage, 
			GetController(), 
			this, 
			UDamageType::StaticClass());
        
	}
}

void AMainPlayer::ProcessAttackHit(const FHitResult& HitResult, float DamageAmount)
{
	if (!HasAuthority()) return;

	AActor* HitActor = HitResult.GetActor();
	// 0. 중복처리
	if (DamagedActors.Contains(HitActor)) return;
	DamagedActors.Add(HitActor);

	// 1. 일반 액터 대미지 처리
	if (HitActor)
	{
		float FinalDamage = 0.0f;
		FItemBaseData HoldingItem = GetHoldingItemReference();
		
		FinalDamage = UStatusComponent::CalculateFinalDamage(this, HitActor, DamageAmount);
		UE_LOG(LogTemp, Warning, TEXT("FinalDamage"));
		if (HoldingItem.IsValid())
		{
			if (InventoryComponent->GetItemData(HoldingItem)->ID == FName(TEXT("EQ004")) && HitActor->IsA(ATree::StaticClass()))
			{
				FinalDamage = UStatusComponent::CalculateTrueDamage(this, HitActor, DamageAmount);
				UE_LOG(LogTemp, Warning, TEXT("TrueDamage"));
			}
		}
		
		
		UGameplayStatics::ApplyDamage(
			HitActor, 
			FinalDamage, 
			GetController(), 
			this, 
			UDamageType::StaticClass());

		if (UStatusComponent* HitStatus = HitActor->FindComponentByClass<UStatusComponent>())
		{
			float RemainingHP = HitStatus->CurrentHP; 

			if (GEngine)
			{
				FString DebugMsg = FString::Printf(TEXT("[%s] 남은 체력: %.1f"), *HitActor->GetName(), RemainingHP);
				//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, DebugMsg);
				FString DebugMsgs = FString::Printf(TEXT("최종데미지: %.1f"), FinalDamage);
				//GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, DebugMsgs);
			}
		}

		if (HoldingItem.IsValid())
		{
			FItemData* ItemData = InventoryComponent->GetItemData(HoldingItem);

			if (ItemData)
			{
				const bool bIsEquipment = (ItemData->Type == EItemType::EQUIPMENT);

				if (bIsEquipment)
				{
					FItemBaseData SlotItem = InventoryComponent->GetItemAtIndex(HotBarIndex);

					SlotItem.CurrentDurability = FMath::Clamp(
						SlotItem.CurrentDurability - 1.0f,
						0.0f,
						SlotItem.MaxDurability
					);

					if (SlotItem.CurrentDurability <= 0.0f)
					{
						InventoryComponent->Request_RemoveItemAmountAtSlot(HotBarIndex, 1);
					}
					else
					{
						InventoryComponent->Request_SetItemAtSlot(HotBarIndex, SlotItem);
					}
				}
			}
		}
                
		UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *HitActor->GetName());
	}
	
	// 2. 폴리지 변환 시도
	TryConvertFoliageToActor(HitResult, DamageAmount);
}

TArray<FString> AMainPlayer::GetRecipeNames() const
{
	TArray<FString> Options;
	Options.Add(TEXT("None"));

	if (RecipeDataTable)
	{
		TArray<FName> RowNames = RecipeDataTable->GetRowNames();
		for (const FName& RowName : RowNames)
		{
			Options.Add(RowName.ToString());
		}
	}

	return Options;
}

bool AMainPlayer::HasRecipe(const FName& RecipeID) const
{
	if (RecipeID.IsNone()) return false;

	// 1. 개인 레시피 확인
	if (AMainPlayerState* PS = GetPlayerState<AMainPlayerState>())
	{
		if (PS->PersonalRecipes.Contains(RecipeID)) return true;
	}

	// 2. 공유 레시피 확인
	if (AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>())
	{
		if (GS->SharedRecipes.Contains(RecipeID)) return true;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("[PLAYER] Player doesn't has Recipe %s"), *RecipeID.ToString())
	return false;
}

void AMainPlayer::StartCraft(FRecipeData RecipeData)
{
	GetWorld()->GetTimerManager().SetTimer(
		CraftTimer,
		[this, RecipeData]()
		{
			InventoryComponent->Request_MakeItem(RecipeData);
		},
		RecipeData.Duration,
		false);
}

void AMainPlayer::StopCraft()
{
	GetWorld()->GetTimerManager().ClearTimer(CraftTimer);
}

void AMainPlayer::Emotion(const FInputActionInstance& Instance)
{
	UE_LOG(LogTemp, Warning, TEXT("[PLAYER] Emotion"));
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		if (PC->IsInputKeyDown(EKeys::One))
		{
			UE_LOG(LogTemp, Warning, TEXT("Ctrl + 1"));
			if (EmotionMontages[0])
			{
				Request_Emotion(EmotionMontages[0]);
			}
		}
		else if (PC->IsInputKeyDown(EKeys::Two))
		{
			UE_LOG(LogTemp, Warning, TEXT("Ctrl + 2"));
			if (EmotionMontages[1])
			{
				Request_Emotion(EmotionMontages[1]);
			}
		}
	}
}

void AMainPlayer::Request_Emotion(UAnimMontage* Emotion)
{
	if (HasAuthority())
	{
		Multi_PlayAnimMontage(Emotion);
	} else
	{
		Server_Emotion(Emotion);
	}
}

void AMainPlayer::Server_Emotion_Implementation(UAnimMontage* Emotion)
{
	Multi_PlayAnimMontage(Emotion);
}

void AMainPlayer::Request_StopEmotion(UAnimMontage* Emotion)
{
	if (HasAuthority())
	{
		Multi_StopAnimMontage(Emotion);
	} else
	{
		Server_StopEmotion(Emotion);
	}
}

void AMainPlayer::Server_StopEmotion_Implementation(UAnimMontage* Emotion)
{
	Multi_StopAnimMontage(Emotion);
}

//멀티플레이어 코드

void AMainPlayer::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMainPlayer, IsRunning);
	DOREPLIFETIME(AMainPlayer, IsCrouching);
	DOREPLIFETIME(AMainPlayer, IsSliding);
	DOREPLIFETIME(AMainPlayer, IsInability);
	DOREPLIFETIME(AMainPlayer, IsInventoryOpen);
	DOREPLIFETIME(AMainPlayer, IsHoldingItem);
	DOREPLIFETIME(AMainPlayer, IsUsingItem);
	DOREPLIFETIME(AMainPlayer, IsAttacking);
	DOREPLIFETIME(AMainPlayer, HotBarIndex);
	DOREPLIFETIME(AMainPlayer, ItemMesh);
	DOREPLIFETIME(AMainPlayer, IsSwimming);
	DOREPLIFETIME(AMainPlayer, MovementMultiplier);
	DOREPLIFETIME(AMainPlayer, CanInteract);
	DOREPLIFETIME(AMainPlayer, CharacterRole);
	DOREPLIFETIME(AMainPlayer, SwimMode);
	DOREPLIFETIME(AMainPlayer, Torch);
	DOREPLIFETIME(AMainPlayer, InteractingPlayer);
}

void AMainPlayer::OnRep_Interacted()
{
	//여기서 HUD로 살리는 중! 보여주기
	//날 만지는 사람이 있다..
	if (InteractingPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PLAYER] SOMEONE TOUCHING ME...(%s)"), *GetName());
		if (HUD)
		{
			HUD->DisplayInteractionInfoText(InteractingPlayer);
		}
	} 
	//이제 아무도 안만짐
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PLAYER] STOP TOUCHING ME...(%s)"), *GetName());
		if (HUD)
		{
			HUD->HideInteractionInfoText();
		}
	}
}

void AMainPlayer::Request_BeginInteractPlayer(AActor* Target)
{
	if (HasAuthority())
	{
		BeginInteractPlayer(Target);
	} else
	{
		Server_BeginInteractPlayer(Target);
	}
}

void AMainPlayer::Server_BeginInteractPlayer_Implementation(AActor* Target)
{
	BeginInteractPlayer(Target);
}

void AMainPlayer::BeginInteractPlayer(AActor* Target)
{
	if (!Target) return;
	
	if (AMainPlayer* NewTarget = Cast<AMainPlayer>(Target))
	{
		if (!NewTarget->IsInability) return;
		
		NewTarget->InteractingPlayer = this;
		NewTarget->OnRep_Interacted();
	}
}

void AMainPlayer::Request_EndInteractPlayer(AActor* Target)
{
	if (HasAuthority())
	{
		EndInteractPlayer(Target);
	} else
	{
		Server_EndInteractPlayer(Target);
	}
}

void AMainPlayer::Server_EndInteractPlayer_Implementation(AActor* Target)
{
	EndInteractPlayer(Target);
}

void AMainPlayer::EndInteractPlayer(AActor* Target)
{
	if (!Target) return;
	
	if (AMainPlayer* NewTarget = Cast<AMainPlayer>(Target))
	{
		if (NewTarget->InteractingPlayer == this)
		{
			NewTarget->InteractingPlayer = nullptr;
			NewTarget->OnRep_Interacted();
		}
	}
	
}

void AMainPlayer::Request_Run()
{
	if (IsBuildingInputBlocked() || IsInability) return;

	if (HasAuthority())
	{
		Run();
	} else
	{
		Server_Run();
	}
}

void AMainPlayer::Request_StopRun()
{
	if (HasAuthority())
	{
		StopRun();	
	} else
	{
		Server_StopRun();
	}
}

void AMainPlayer::Request_ToggleCrouch()
{
	if (IsBuildingInputBlocked()) return;

	if (HasAuthority())
	{
		ToggleCrouch();
	} else
	{
		Server_ToggleCrouch();
	}
}

void AMainPlayer::OnRep_IsCrouching()
{
	if (IsCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
	} else
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

void AMainPlayer::OnRep_IsInability()
{
	if (IsInability)
	{
		GetCharacterMovement()->MaxWalkSpeed = KnockOutSpeed;
	} else
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

void AMainPlayer::OnRep_IsRunning()
{
	if (IsRunning)
	{
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	} else
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

void AMainPlayer::Request_Attack()
{
	if (IsBuildingInputBlocked()) return;

	if (UBuildingComponent* BuildComp = FindComponentByClass<UBuildingComponent>())
	{
		if (BuildComp->GetCurrentState() == EBuildingState::Placing)
		{
			BuildComp->ConfirmBuild(); 
			return;
		}
	}
	if (HasAuthority())
	{
		Attack();
	} else
	{
		Server_Attack();
	}
}

void AMainPlayer::Request_RefreshHand()
{
	if (HasAuthority())
	{
		RefreshHand();
	} else
	{
		Server_RefreshHand();
	}
}

void AMainPlayer::Request_SetHotbarIndex(int32 Index)
{
	if (HasAuthority())
	{
		SetHotbarIndex(Index);
		if (HUD) HUD->UpdateHotBar();
		RefreshHand();
	} else
	{
		Server_SetHotbarIndex(Index);
	}
}

void AMainPlayer::Server_SetHotbarIndex_Implementation(int32 Index)
{
	SetHotbarIndex(Index);
	RefreshHand();
}

void AMainPlayer::OnRep_HotBarIndex()
{
	if (IsLocallyControlled())
	{
		if (HUD)
		{
			HUD->UpdateHotBar();
		}
	}
	RefreshHand();
}

void AMainPlayer::OnRep_HandedItem()
{
	
}

void AMainPlayer::Request_DropItem(UInventoryComponent* SourceInventory, int32 SourceIndex, int32 AmountToDrop, bool IsWhole)
{
	if (HasAuthority())
	{
		DropItem(SourceInventory, SourceIndex, AmountToDrop, IsWhole);
	} else
	{
		Server_DropItem(SourceInventory, SourceIndex, AmountToDrop, IsWhole);
	}
}

void AMainPlayer::Request_StartUseItem()
{
	if (UBuildingComponent* BuildComp = FindComponentByClass<UBuildingComponent>())
	{
		const EBuildingState State = BuildComp->GetCurrentState();
		if (State == EBuildingState::Placing || State == EBuildingState::Building)
		{
			BuildComp->CancelBuild();
			
			return; 
		}
	}

	if (IsBuildingInputBlocked()) return;
	
	if (HasAuthority())
	{
		StartUseItem();
	} else
	{
		Server_StartUseItem();
	}
}

void AMainPlayer::Server_StartUseItem_Implementation()
{
	StartUseItem();
}

void AMainPlayer::Request_StopUseItem()
{
	if (HasAuthority())
	{
		StopUseItem();
	} else
	{
		Server_StopUseItem();
	}
}

void AMainPlayer::Request_StartCraft(FRecipeData RecipeData)
{
	if (HasAuthority())
	{
		StartCraft(RecipeData);
	} else
	{
		Server_StartCraft(RecipeData);
	}
}

void AMainPlayer::Server_StartCraft_Implementation(FRecipeData RecipeData)
{
	StartCraft(RecipeData);
}

void AMainPlayer::Request_StopCraft()
{
	if (HasAuthority())
	{
		StopCraft();
	} else
	{
		Server_StopCraft();
	}
}

void AMainPlayer::Multi_StopAnimMontage_Implementation(UAnimMontage* Anim)
{
	StopAnimMontage(Anim);
}

void AMainPlayer::Server_PlayHealMontage_Implementation()
{
	if (HealMontage)
	{
		Multi_PlayAnimMontage(HealMontage);
	}
}

void AMainPlayer::Server_StopHealMontage_Implementation()
{
	if (HealMontage)
	{
		Multi_StopAnimMontage(HealMontage);
	}
}

void AMainPlayer::FoundInteractableWater(UPrimitiveComponent* WaterComp)
{
	if (InteractionData.CurrentInteractable || InteractionData.CurrentFoliageComponent)
	{
		NotFoundInteractable(); 
	}

	InteractionData.CurrentWaterComponent = WaterComp;
}

void AMainPlayer::Server_DrinkWater_Implementation(UPrimitiveComponent* WaterComp)
{
	if (!WaterComp || !StatusComponent) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime < LastDrinkTime + DrinkCooldown) return;
    
	AActor* WaterActor = WaterComp->GetOwner();
	if (!WaterActor) return;

	FString ClassName = WaterActor->GetClass()->GetName();

	if (ClassName.Contains(TEXT("WaterBodyRiver")) || ClassName.Contains(TEXT("WaterBodyLake")))
	{
		bool bFilledBottle = false;

		FItemBaseData HoldingItem = GetHoldingItemReference();
		//빈 물병 들고 있으면
		if (HoldingItem.IsValid() && HoldingItem.ItemID == FName("FQ005"))
		{
			if (InventoryComponent)
			{
				InventoryComponent->Request_RemoveItemAmountAtSlot(HotBarIndex, 1);
				
				//빈 물병 지우고 찬 물병으로 넣기
				FItemBaseData FilledWaterBottle = InventoryComponent->CreateItemByID(FName("FO019"), 1);
				if (FilledWaterBottle.IsValid())
				{
					InventoryComponent->Request_HandleAddItem(FilledWaterBottle);
				}
				
				bFilledBottle = true;
			}
		}
		
		if (bFilledBottle == false)
		{
			StatusComponent->IncreaseHydration(5.0f);
			
			if (DrinkMontage)
			{
				Multi_PlayAnimMontage(DrinkMontage);
			}
		}
		
		LastDrinkTime = CurrentTime;
	}
	else if (ClassName.Contains(TEXT("WaterBodyOcean")))
	{
		StatusComponent->DecreaseHydration(5.0f);
		
		if (DrinkMontage)
		{
			Multi_PlayAnimMontage(DrinkMontage);
		}

		LastDrinkTime = CurrentTime;
	}
}

void AMainPlayer::Client_ShowDeathScreen_Implementation()
{
	//로컬 화면 흑백으로 전환
	if (IsLocallyControlled())
	{
		FirstPersonCamera->PostProcessSettings.bOverride_ColorSaturation = true;
		FirstPersonCamera->PostProcessSettings.ColorSaturation = FVector4(0,0,0,0);
		
		if (InventoryWidget) InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
		
		if (AMainPlayerController* MPC = GetController<AMainPlayerController>())
		{
			MPC->OpenDeathScreen();
		} else
		{
			UE_LOG(LogTemp, Warning, TEXT("Main Player Controller Not Connected"));	
		}
	}
}

void AMainPlayer::Server_StopCraft_Implementation()
{
	StopCraft();
}

void AMainPlayer::Multi_PlayAnimMontage_Implementation(UAnimMontage* Anim)
{
	PlayAnimMontage(Anim);
}

void AMainPlayer::Multi_PlayForcedRestStart_Implementation()
{
	if (!SitDownMontage) return;

	// 주저앉기 = 일어서는 모션을 PlayRate -1로 역재생
	PlayAnimMontage(SitDownMontage, -1.0f);

	// 몽타주가 끝났을 때(역재생 완료 = 앉은 자세) 마지막 프레임에서 멈추도록 바인딩
	if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
	{
		// 중복 바인딩 방지
		AnimInst->OnMontageEnded.RemoveDynamic(this, &AMainPlayer::OnSitDownMontageEnded);
		AnimInst->OnMontageEnded.AddDynamic(this, &AMainPlayer::OnSitDownMontageEnded);
	}
}

void AMainPlayer::OnSitDownMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != SitDownMontage) return;

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (!AnimInst) return;

	// 콜백 제거
	AnimInst->OnMontageEnded.RemoveDynamic(this, &AMainPlayer::OnSitDownMontageEnded);

	// 중단(인터럽트)된 게 아니고 아직 강제 휴식 중이면 앉은 포즈 유지
	if (!bInterrupted && StatusComponent && StatusComponent->bIsForcedResting)
	{
		// 이미 끝난 몽타주는 Pause가 무시되므로:
		// position 0.0(앉은 포즈)에서 다시 Play한 뒤 즉시 Pause → 포즈 고정
		AnimInst->Montage_Play(SitDownMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
		AnimInst->Montage_Pause(SitDownMontage);
	}
}

void AMainPlayer::Multi_PlayForcedRestEnd_Implementation()
{
	// PlayAnimMontage가 내부적으로 기존 몽타주(일시정지된 SitDown)를 교체하므로
	// 별도로 Montage_Stop 호출 불필요 → 그 사이 Idle 프레임 방지
	if (StandUpMontage)
	{
		PlayAnimMontage(StandUpMontage, 1.0f);
	}
}

void AMainPlayer::Multi_PlaySoundAtLocation_Implementation(USoundBase* Sound, FVector Location)
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location);
}

void AMainPlayer::Server_StopUseItem_Implementation()
{
	StopUseItem();
}

void AMainPlayer::Client_PlaySound2D_Implementation(USoundBase* Sound)
{
	UGameplayStatics::PlaySound2D(GetWorld(), Sound);
}

void AMainPlayer::Client_OpenBonfireUI_Implementation()
{
	if (BonfireUIClass)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (!PC) return;

		UBonFireUI* BonfireWidget = CreateWidget<UBonFireUI>(PC, BonfireUIClass);
		if (BonfireWidget)
		{
			BonfireWidget->AddToViewport();

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(BonfireWidget->TakeWidget());
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
		}
	}
}

void AMainPlayer::Server_DropItem_Implementation(UInventoryComponent* SourceInventory, int32 SourceIndex, int32 AmountToDrop, bool IsWhole)
{
	DropItem(SourceInventory, SourceIndex, AmountToDrop, IsWhole);
}

void AMainPlayer::Server_ToggleCrouch_Implementation()
{
	ToggleCrouch();
}

void AMainPlayer::Server_Run_Implementation()
{
	Run();
}

void AMainPlayer::Server_StopRun_Implementation()
{
	StopRun();	
}

void AMainPlayer::Server_Attack_Implementation()
{
	Attack();
}

void AMainPlayer::Server_RefreshHand_Implementation()
{
	RefreshHand();
}

void AMainPlayer::Multi_RemoveFoliageInstance_Implementation(UInstancedStaticMeshComponent* ISMC, int32 InstanceIndex)
{
	if (ISMC)
	{
		ISMC->RemoveInstance(InstanceIndex);
	}
}

void AMainPlayer::Client_OpenRepairUI_Implementation(class ARepair_Actor* TargetActor)
{
	if (RepairUIClass && TargetActor)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (!PC) return;

		// OwningObject를 PlayerController로 변경
		URepairUI* RepairWidget = CreateWidget<URepairUI>(PC, RepairUIClass);
		if (RepairWidget)
		{
			RepairWidget->InitRepairWindow(TargetActor);
			RepairWidget->AddToViewport();

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(RepairWidget->TakeWidget());
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
		}
	}
}
