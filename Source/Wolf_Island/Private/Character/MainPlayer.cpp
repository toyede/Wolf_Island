// Fill out your copyright notice in the Description page of Project Settings.


#include "Wolf_Island/Public/Character/MainPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "MaterialHLSLTree.h"
#include "Engine/DamageEvents.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Craft/BonFireUI.h"
#include "Widgets/Craft/RepairUI.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/StatusComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/WeaponComponent.h"
#include "Item/Tree.h"
#include "Components/CapsuleComponent.h"
#include "Components/InventoryComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Games/MainHUD.h"
#include "Interaction/InteractionInterface.h"
#include "Item/Pickup.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/PlayerHUD.h"
#include "BuoyancyComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BillboardComponent.h"
#include "WaterBodyComponent.h"

// Sets default values
AMainPlayer::AMainPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StatusComponent = CreateDefaultSubobject<UStatusComponent>("StatusComponent");

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("InventoryComponent");

	WeaponComponent = CreateDefaultSubobject<UWeaponComponent>("WeaponComponent");
	
	BuoyancyComponent = CreateDefaultSubobject<UBuoyancyComponent>("BuoyancyComponent");
	
	WaterLevelCheckPoint = CreateDefaultSubobject<UBillboardComponent>("WaterLevelCheckPoint");
	
	WaterAmbience = CreateDefaultSubobject<UAudioComponent>("WaterAmbience");

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
	InventoryComponent->SetWeightCapacity(StatusComponent->MaxWeight);

	GetCharacterMovement()->SetIsReplicated(true);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	
	//수영을 위한 부력 컴포넌트 세팅
	WaterLevelCheckPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
	BuoyancyComponent->AddCustomPontoon(25.0f, WaterLevelCheckPoint->GetRelativeLocation());
	
	GetCharacterMovement()->MaxFlySpeed = SwimmingSpeed;
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

			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(BonfireWidget->TakeWidget());
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
		}
	}
}

// Called when the game starts or when spawned
void AMainPlayer::BeginPlay()
{
	Super::BeginPlay();
	
	InteractableData.InteractionDuration = InteractionDuration;
	
	if(StatusComponent){
		if (HasAuthority())
		{
			//상태 델리게이트 바인딩
			StatusComponent->OnStaminaZero.AddDynamic(this, &AMainPlayer::Request_StopRun);

			//죽음 바인딩
			StatusComponent->OnHPZero.AddDynamic(this, &AMainPlayer::OnDeath);

			//배고픔, 수분 감소 시작
			StatusComponent->StartHunger();
			StatusComponent->StartHydration();
		}
		
		//산소 게이지 숨기기 바인딩(테스트용)
		StatusComponent->OnAirFull.AddDynamic(this, &AMainPlayer::HideAirBar);
	}

	if (InventoryComponent)
	{
		//아이템 업데이트 바인딩
		InventoryComponent->OnInventoryUpdated.AddUObject(this, &AMainPlayer::RefreshHand);
	}

	if (WeaponComponent)
	{
		RefreshHand();
	}
	
	if (BuoyancyComponent)
	{
		BuoyancyComponent->OnEnteredWaterDelegate.AddDynamic(this, &AMainPlayer::EnterWater);
		BuoyancyComponent->OnExitedWaterDelegate.AddDynamic(this, &AMainPlayer::ExitWater);
	}

	//HUD = Cast<AMainHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());

	//플레이어 본인만 HUD 생성.
	if (IsLocallyControlled())
	{
		HUD = CreateWidget<UPlayerHUD>(GetWorld(), HUDClass);
		HUD->AddToViewport();
	}
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
			FVector Curr;

			if (WeaponComponent->IsEquipped)
			{
				Curr = ItemMesh->GetSocketLocation(TracePoint.Key);
			}
			else
			{
				Curr = GetMesh()->GetSocketLocation(TracePoint.Key);
			}
			
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

void AMainPlayer::StartJump()
{
	//스태미나가 0이면 점프 불가
	if (StatusComponent->CurrentStamina <= JumpConsumeAmount) return;

	//낙하 중(점프 중) 이면 점프 불가
	if (GetCharacterMovement()->IsFalling()) return;

	//슬라이딩 중이면 점프 불가
	if (IsSliding) return;
	
	//달리는 중 점프하면 스태미나 감소 중단
	if (IsRunning)
	{
		StatusComponent->StopStamina();
	}

	//점프 시 스태미나 회복 중단
	StatusComponent->StopRecoverStamina();
	//점프 스태미나 소모
	StatusComponent->DecreaseStamina(JumpConsumeAmount);

	if (JumpSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), JumpSound, GetActorLocation());
	}
	
	Jump();
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
			StatusComponent->DecreaseHP(StatusComponent->MaxHP);
		} else
		{
			StatusComponent->DecreaseHP(FallForce*0.03f);
		}
	}

	//달리는 중이면 스태미나 감소 시작
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

//시야 함수
void AMainPlayer::Look(const FInputActionValue& Value)
{
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
	FVector2D MovementVector = Value.Get<FVector2D>();
	//UE_LOG(LogTemp, Warning, TEXT("MOVE X: %f, Y: %f"), MovementVector.X, MovementVector.Y);

	if (Controller)
	{
		if (IsSwimming)
		{
			FVector ForwardVector = UKismetMathLibrary::GetForwardVector(GetControlRotation());
			AddMovementInput(GetActorRightVector(), MovementVector.X*WaterDeceleration);
			AddMovementInput(ForwardVector, MovementVector.Y*WaterDeceleration);
		} else
		{
			AddMovementInput(GetActorRightVector(), MovementVector.X);
			AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		}
	}
}

//Shift 누른 상태로 Run -> 스태미나 소진 -> Run 상태 유지
//특정 시간 후 스태미나 회복 -> Shift 떼면 스태미나 소진
void AMainPlayer::Run()
{	
	//속도가 있는가? -> 뛰는 중인가?
	if (GetVelocity().Size() > 0){

		//낙하 중이면 달리기 불가
		if (GetMovementComponent()->IsFalling())
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
	//웅크리는 중이면
	if (IsCrouching)
	{
		UnCrouch();
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		IsCrouching = false;
	} else
	{
		Crouch();
		GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
		IsCrouching = true;
	}
}

void AMainPlayer::ToggleInventory()
{
	//인벤토리가 열려 있으면
	if (IsInventoryOpen)
	{
		IsInventoryOpen = false;
	}
	//인벤토리가 닫혀 있으면
	else
	{
		IsInventoryOpen = true;
	}
}

//TODO: 아이템 사용 로직 멀티로 전환하기
void AMainPlayer::UseItem(int32 SlotIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("USE ITEM EXECUTED"));
	
	if (InventoryComponent)
	{
		FItemData* ItemData = InventoryComponent->GetItemDataAtIndex(SlotIndex);
		
		if (ItemData->IsNotEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("TRY TO USE THIS : [ %s ]"), *ItemData->TextData.Name.ToString());
			
			if (StatusComponent && ItemData->Type == EItemType::FOOD)
			{
				if (ItemData->Type == EItemType::FOOD && EatingSound)
				{
					Multi_PlaySound(EatingSound, GetActorLocation());
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
			//사용까지 꾹 눌러야 하는 시간
			float UseDuration = ItemData->NumericData.UseDuration;
		
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
	UE_LOG(LogTemp, Display, TEXT("Player Dead"));
}

//손에 든 아이템 업데이트 함수
void AMainPlayer::RefreshHand()
{
	//핫바 인덱스의 아이템 정보 가져오기.
	FItemBaseData Item = InventoryComponent->GetItemAtIndex(HotBarIndex);
	
	//해당 인덱스 인벤토리 칸에 아이템이 있으면 그 아이템 들기.
	if (Item.IsValid())
	{	
		//데이터 베이스에서 아이템 데이터 가져오기
		FItemData* ItemData = InventoryComponent->GetItemData(Item);
		
		IsHoldingItem = true;
		ItemMesh->SetStaticMesh(ItemData->AssetData.Mesh);
		ItemMesh->AttachToComponent(
		GetMesh(),
		FAttachmentTransformRules::KeepRelativeTransform,
		TEXT("hand_r"));
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		
		FTransform SocketTransform = ItemMesh->GetSocketTransform(TEXT("HandSocket"), RTS_Component);
		ItemMesh->SetRelativeTransform(SocketTransform.Inverse());

		//ItemMesh->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
		WeaponComponent->CheckWeapon(Item);
	} else
	{
		WeaponComponent->CheckWeapon(Item);
		IsHoldingItem = false;
		ItemMesh->SetStaticMesh(nullptr);
	}
}

void AMainPlayer::Attack()
{
	if (IsSwimming) return;
	
	WeaponComponent->Request_UseWeapon();
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
		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			//부딪힌 액터가 인터랙션 인터페이스를 가지고 있나?
			if (HitResult.GetActor()->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
			{
				//UE_LOG(LogTemp, Warning, TEXT("It has interface."));
				//부딪힌 액터가 현재 인터랙터블 데이터와 다르다면
				if (HitResult.GetActor() != InteractionData.CurrentInteractable)
				{
					//UE_LOG(LogTemp, Warning, TEXT("FoundInteractable"));
					//TargetInteractable에 결과물 넣기
					FoundInteractable(HitResult.GetActor());
					return;
				}

				//부딪힌 액터가 현재 인터랙터블 액터와 같다면 암것두 안하기~
				if (HitResult.GetActor() == InteractionData.CurrentInteractable)
				{
					return;
				}
			}
		}
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("NO CAMERA"));
	}
	NotFoundInteractable();
}

void AMainPlayer::FoundInteractable(AActor* Interactable)
{
	if (IsInteracting()) 
	{
		EndInteract();
	}
	
	//현재 인터랙션 액터 데이터가 있으면
	if (InteractionData.CurrentInteractable)
	{	
		TargetInteractionInterface = InteractionData.CurrentInteractable;
		TargetInteractionInterface->EndFocus();
	}
	
	//인터랙션 액터 데이터 지정
	InteractionData.CurrentInteractable = Interactable;
	TargetInteractionInterface = Interactable;
	
	//인터랙터블 액터의 상태가 인터랙션 가능한 상태가 아니면
	if (!TargetInteractionInterface->InteractableData.CanInteract)
	{
		//TODO:여기 인터랙션 UI 해제 코드 추가 예정
		if (IsLocallyControlled())
		{
			HUD->DisplayDefault();
		}
		TargetInteractionInterface->EndFocus();
		return;
	}
	
	//TODO:여기 인터랙션 UI 업데이트 코드 추가 예정
	if (IsLocallyControlled())
	{
		HUD->DisplayInteractable();
	}
	TargetInteractionInterface->BeginFocus();
}

//인터랙션 가능 액터를 못찾았을 때
void AMainPlayer::NotFoundInteractable()
{
	//인터랙션 중이면
	if (IsInteracting())
	{
		GetWorldTimerManager().ClearTimer(InteractionTimer);
	}

	//인터랙션 액터 데이터가 있으면
	if (InteractionData.CurrentInteractable) 
	{
		//그 액터가 아직 유효한 액터면
		if (IsValid(TargetInteractionInterface.GetObject()))
		{
			//포커스 끝내기
			TargetInteractionInterface->EndFocus();
		}

		//TODO:여기 인터랙션 UI 업데이트 코드 추가 예정
		if (IsLocallyControlled())
		{
			HUD->DisplayDefault();
		}
		
		//인터랙션 액터 데이터 비우기
		InteractionData.CurrentInteractable = nullptr;
		TargetInteractionInterface = nullptr;
	}
}

//인터랙션 시작 함수 (인터랙션 키 눌렀을 때)
void AMainPlayer::BeginInteract()
{
	IInteractionInterface::BeginInteract();

	//인터랙션이 시작됐을 때부터 인터렉션 상태가 변하지 않는 것을 체크
	CheckInteraction();

	//인터랙션 데이터가 있으면
	if (InteractionData.CurrentInteractable)
	{
		//인터랙션 액터가 유효하면
		if (IsValid(TargetInteractionInterface.GetObject()))
		{
			//인터랙션 타겟 액터
			AActor* Target = Cast<AActor>(TargetInteractionInterface.GetObject());
			//인터랙션 액터의 인터랙션 시작 함수 실행
			TargetInteractionInterface->BeginInteract();
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
				HUD->DisplayInteraction();
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

void AMainPlayer::EndInteract()
{
	IInteractionInterface::EndInteract();
	HUD->HideInteraction();
	//인터랙션 타이머 클리어
	GetWorldTimerManager().ClearTimer(InteractionTimer);

	//인터랙션 액터가 유효한 지 체크
	if (IsValid(TargetInteractionInterface.GetObject()))
	{
		//인터랙션 액터의 인터랙션 종료 함수 실행
		TargetInteractionInterface->EndInteract();
	}
}


void AMainPlayer::Interaction_Implementation(AActor* Target)
{
	UE_LOG(LogTemp, Warning, TEXT("[%hs] SERVER INTERACTION EXECUTED"), HasAuthority()?"SERVER":"CLIENT")
	//인터랙션 타이머 클리어
	GetWorldTimerManager().ClearTimer(InteractionTimer);
	if (IsLocallyControlled())
	{
		HUD->HideInteraction();
	}
	
	//인터랙션 액터가 유효한 지 체크
	if (IsValid(Target))
	{
		TargetInteractionInterface = Target;	
		//인터랙션 액터가 인터랙션 가능한 상태이면
		if (TargetInteractionInterface->InteractableData.CanInteract)
		{
			//인터랙션 액터의 인터랙션 함수 실행
			TargetInteractionInterface->Interact(this);
		}
	}
}

void AMainPlayer::DropItem(UInventoryComponent* SourceInventory, int32 SourceIndex, int32 AmountToDrop, bool IsWhole)
{
	FItemBaseData ItemData = SourceInventory->GetInventory()[SourceIndex].ItemData;
	
	//아이템 데이터가 있으면
	if (ItemData.IsValid())
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
		
		APickup* Pickup = GetWorld()->SpawnActor<APickup>(APickup::StaticClass(), SpawnTransform, SpawnParams);
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
	//트레이스 파라미터 설정
	ETraceTypeQuery TraceTypeQuery = UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel4);
	//ECC_GameTraceChannel4 <- Weapon 채널
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);
	FHitResult Hit;

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
	//라인 트레이스 실행
	/*if (UKismetSystemLibrary::LineTraceSingle(
	GetWorld(),
	StartPos,
	EndPos,
	TraceTypeQuery,
	true,
	IgnoreActors,
	//EDrawDebugTrace::None,
	EDrawDebugTrace::ForDuration,
	Hit,
	true))*/
	{
		//맞은 액터
		AActor* HitActor = Hit.GetActor();
		FItemBaseData HoldingItem = GetHoldingItemReference();

		//기본 대미지
		float Damage = 10.0f;

		//무기 장착 시 무기 대미지로 설정
		if (HoldingItem.IsValid())
		{
			FItemData* ItemData = InventoryComponent->GetItemData(HoldingItem);
			Damage = ItemData->NumericData.Damage;
		}

		//최초로 맞고 또 맞은 액터면 무시
		if (DamagedActors.Contains(HitActor)) return;

		//최초로 맞은 액터면 맞은 액터 배열에 추가
		DamagedActors.Add(HitActor);
		
		//대미지 적용
		UGameplayStatics::ApplyDamage(
			HitActor,
			Damage,
			GetController(),
			this,
			UDamageType::StaticClass());
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
			UE_LOG(LogTemp, Warning, TEXT("Find ItemMesh Points"));
			//있는 히트 포인트만 넣기
			if (ItemMesh->DoesSocketExist(Socket))
			{
				FVector Pos = ItemMesh->GetSocketLocation(Socket);
				TracePoints.Add(Socket, { Pos, Pos });
			}
		}
		//맨손이면 손의 히트 포인트 찾기
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Find Character Points"));
			if (GetMesh()->DoesSocketExist(Socket))
			{
				UE_LOG(LogTemp, Warning, TEXT("FIND POINT [ %s ] ON CHARACTER"), *Socket.ToString());
				FVector Pos = GetMesh()->GetSocketLocation(Socket);
				TracePoints.Add(Socket, { Pos, Pos });
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
	FItemBaseData Item = InventoryComponent->GetItemAtIndex(HotBarIndex);
	if (Item.IsValid())
	{
		Request_DropItem(InventoryComponent, HotBarIndex, 1, true);
	}
}

void AMainPlayer::WaterElevation(const FInputActionValue& Value)
{
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
	if (IsLocallyControlled())
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
	UE_LOG(LogTemp, Warning, TEXT("Surface Z : %f"), SurfaceLocation.Z);
	UE_LOG(LogTemp, Warning, TEXT("WAVE Z : %f"), WaveInfo.Height);
	
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
				HUD->DisplayAirBar();
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
				HUD->DisplayAirBar();
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

void AMainPlayer::TryConvertFoliageToActor(const FHitResult& HitResult, float DamageAmount)
{
	// 1. 맞은 컴포넌트가 유효한지 먼저 확인
	UPrimitiveComponent* HitComponent = HitResult.GetComponent();
	if (!HitComponent) return;

	// 2. 폴리지(InstancedStaticMeshComponent)인지 변환 시도
	UInstancedStaticMeshComponent* ISMC = Cast<UInstancedStaticMeshComponent>(HitComponent);

	// [중요] 폴리지가 아니면(nullptr이면) 여기서 즉시 함수 종료! (땅바닥 등을 쳤을 때 크래시 방지)
	if (!ISMC) 
	{
		// 디버깅용 메시지 (필요 없으면 주석 처리)
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("맞은 건 폴리지가 아님"));
		return; 
	}

	// 3. 어떤 나무(StaticMesh)인지 확인
	UStaticMesh* HitMesh = ISMC->GetStaticMesh();

	// 4. 맵(목록)에 등록된 나무인지 확인
	// HitMesh가 없거나, 맵에 등록되지 않은 풀/돌멩이라면 무시
	if (!HitMesh || !FoliageToActorMap.Contains(HitMesh)) 
	{
		return;
	}

	// 5. 인덱스 확인 (가끔 -1이 들어오는 경우 방지)
	int32 InstanceIndex = HitResult.Item;
	if (InstanceIndex == INDEX_NONE) return;

	// --- 검증 끝, 변환 시작 ---

	TSubclassOf<ATree> TargetActorClass = FoliageToActorMap[HitMesh];
	if (!TargetActorClass) return;

	FTransform InstanceTransform;
	// 월드 좌표 기준으로 트랜스폼 가져오기
	ISMC->GetInstanceTransform(InstanceIndex, InstanceTransform, true);

	// 폴리지 삭제
	ISMC->RemoveInstance(InstanceIndex);

	// 진짜 액터 소환
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATree* NewTree = GetWorld()->SpawnActor<ATree>(TargetActorClass, InstanceTransform, SpawnParams);

	// 데미지 전달
	if (NewTree)
	{
		FDamageEvent DamageEvent;
		NewTree->TakeDamage(DamageAmount, DamageEvent, GetController(), this);
        
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("나무 변환 성공!"));
	}
}

void AMainPlayer::ProcessAttackHit(const FHitResult& HitResult, float DamageAmount)
{
	AActor* HitActor = HitResult.GetActor();

	if (HitActor)
	{
		UGameplayStatics::ApplyDamage(HitActor, DamageAmount, GetController(), this, UDamageType::StaticClass());
	}

	TryConvertFoliageToActor(HitResult, DamageAmount);
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

			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(RepairWidget->TakeWidget());
			PC->SetInputMode(InputMode);
			PC->bShowMouseCursor = true;
		}
	}
}

//멀티플레이어 코드

void AMainPlayer::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMainPlayer, IsRunning);
	DOREPLIFETIME(AMainPlayer, IsCrouching);
	DOREPLIFETIME(AMainPlayer, IsSliding);
	DOREPLIFETIME(AMainPlayer, IsInability);
	DOREPLIFETIME(AMainPlayer, AttackConsumeAmount);
	DOREPLIFETIME(AMainPlayer, SlideConsumeAmount);
	DOREPLIFETIME(AMainPlayer, IsInventoryOpen);
	DOREPLIFETIME(AMainPlayer, IsHoldingItem);
	DOREPLIFETIME(AMainPlayer, IsAttacking);
	DOREPLIFETIME(AMainPlayer, HotBarIndex);
	DOREPLIFETIME(AMainPlayer, StatusComponent);
	DOREPLIFETIME(AMainPlayer, InventoryComponent);
	DOREPLIFETIME(AMainPlayer, WeaponComponent);
	DOREPLIFETIME(AMainPlayer, ItemMesh);
	DOREPLIFETIME(AMainPlayer, IsSwimming);
}

void AMainPlayer::Request_Run()
{
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
		GetCharacterMovement()->MaxWalkSpeed = 150.0f;
	} else
	{
		GetCharacterMovement()->MaxWalkSpeed = 300.0f;
	}
}

void AMainPlayer::OnRep_IsRunning()
{
	if (IsRunning)
	{
		GetCharacterMovement()->MaxWalkSpeed = 750.0f;
	} else
	{
		GetCharacterMovement()->MaxWalkSpeed = 300.0f;
	}
}

void AMainPlayer::Request_Attack()
{
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
		HUD->UpdateHotBar();
		RefreshHand();
		FItemBaseData Item = InventoryComponent->GetItemAtIndex(HotBarIndex);
		WeaponComponent->CheckWeapon(Item);
	} else
	{
		Server_SetHotbarIndex(Index);
	}
}

void AMainPlayer::Server_SetHotbarIndex_Implementation(int32 Index)
{
	SetHotbarIndex(Index);
	RefreshHand();
	FItemBaseData Item = InventoryComponent->GetItemAtIndex(HotBarIndex);
	WeaponComponent->CheckWeapon(Item);
}

void AMainPlayer::OnRep_HotBarIndex()
{
	HUD->UpdateHotBar();
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

void AMainPlayer::Multi_PlayAnimMontage_Implementation(UAnimMontage* Anim)
{
	PlayAnimMontage(Anim);
}

void AMainPlayer::Multi_PlaySound_Implementation(USoundBase* Sound, FVector Location)
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