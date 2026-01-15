// Fill out your copyright notice in the Description page of Project Settings.


#include "Wolf_Island/Public/Character/MainPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "MaterialHLSLTree.h"
#include "Engine/DamageEvents.h"
#include "Blueprint/UserWidget.h"
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
#include "Item/ItemBase.h"
#include "Item/Pickup.h"
#include "Widgets/PlayerHUD.h"


// Sets default values
AMainPlayer::AMainPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StatusComponent = CreateDefaultSubobject<UStatusComponent>("StatusComponent");

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>("FirstPersonCamera");

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("InventoryComponent");

	WeaponComponent = CreateDefaultSubobject<UWeaponComponent>("WeaponComponent");

	//손에 든 아이템 메쉬
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>("Item");
	//손 소켓에 부-착!
	ItemMesh->SetupAttachment(GetMesh(), "hand_r");
	
	GetMesh()->SetRelativeTransform(
		FTransform(
			FRotator(0, -90, 0),
			FVector(0,0,-90)
			));
	
	//메시에 카메라 붙이기
	//FirstPersonCamera->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, "headSocket");
	//컨트롤러 마우스 위치 입력을 카메라 입력에 반영
	FirstPersonCamera->SetupAttachment(GetMesh());
	FirstPersonCamera->bUsePawnControlRotation = true;
	
	FirstPersonCamera->SetRelativeTransform(
		FTransform(
			FRotator(-90, 90, 90),
			FVector(0,10,0)
			));

	//인벤토리 초기화
	InventoryComponent->SetSlotsCapacity(30);
	InventoryComponent->SetWeightCapacity(StatusComponent->MaxWeight);
}

// Called when the game starts or when spawned
void AMainPlayer::BeginPlay()
{
	Super::BeginPlay();
	
	InteractableData.InteractionDuration = InteractionDuration;
	
	if(StatusComponent){
		//상태 델리게이트 바인딩
		StatusComponent->OnStaminaZero.AddDynamic(this, &AMainPlayer::StopRun);

		//죽음 바인딩
		StatusComponent->OnHPZero.AddDynamic(this, &AMainPlayer::OnDeath);

		//배고픔, 수분 감소 시작
		StatusComponent->StartHunger();
		StatusComponent->StartHydration();
	}

	if (InventoryComponent)
	{
		//아이템 업데이트 바인딩
		//InventoryComponent->OnInventoryUpdated.AddUObject(this, &AMainPlayer::RefreshHand);
	}

	if (WeaponComponent)
	{
		//RefreshHand();
	}

	//HUD = Cast<AMainHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());

	HUD = CreateWidget<UPlayerHUD>(GetWorld(), HUDClass);
	HUD->AddToViewport();
}

// Called every frame
void AMainPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckFrequency)
	{
		CheckInteraction();
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
		
		// 시야
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered,
			this, &AMainPlayer::Look);

		//달리기
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Triggered,
			this, &AMainPlayer::Run);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed,
			this, &AMainPlayer::StopRun);

		//웅크리기
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started,
			this, &AMainPlayer::ToggleCrouch);

		//슬라이딩
		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Started,
			this, &AMainPlayer::Sliding);
		
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
			this, &AMainPlayer::StartUseItem);
		EnhancedInputComponent->BindAction(UseItemAction, ETriggerEvent::Completed,
			this, &AMainPlayer::StopUseItem);

		//핫바 숫자키
		EnhancedInputComponent->BindAction(HotBarAction, ETriggerEvent::Triggered,
			this, &AMainPlayer::HandleHotBar);
		//핫바 마우스 휠
		EnhancedInputComponent->BindAction(HotBarWheelAction, ETriggerEvent::Triggered,
			this, &AMainPlayer::HandleHotBarWithWheel);

		//공격
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started,
			this, &AMainPlayer::Attack);
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

	if (Controller != nullptr)
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

	if (Controller != nullptr)
	{
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

//Shift 누른 상태로 Run -> 스태미나 소진 -> Run 상태 유지
//특정 시간 후 스태미나 회복 -> Shift 떼면 스태미나 소진
void AMainPlayer::Run()
{	
	if (!HasAuthority())
	{
		Server_Run();
	} else
	{
		Multi_Run();
	}
}

void AMainPlayer::StopRun()
{
	if (!HasAuthority())
	{
		Server_StopRun();
	} else
	{
		Multi_StopRun();
	}
}

void AMainPlayer::ToggleCrouch_Implementation()
{
	/*//웅크리는 중이면
	if (IsCrouching)
	{
		UnCrouch();
		GetCharacterMovement()->MaxWalkSpeed = 300.0f;
		IsCrouching = false;
	} else
	{
		Crouch();
		GetCharacterMovement()->MaxWalkSpeed = 150.0f;
		IsCrouching = true;
	}*/
	
	//클라이언트 실행
	if (!HasAuthority())
	{
		Server_ToggleCrouch();
	}
	//서버 실행
	else
	{
		Multi_ToggleCrouch();
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

void AMainPlayer::Sliding()
{
	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();

	if (AnimInst&&!IsSliding)
	{
		StatusComponent->DecreaseStamina(SlideConsumeAmount);
		IsSliding = true;
		GetCapsuleComponent()->SetCapsuleHalfHeight(30);
		GetMesh()->SetRelativeLocation(FVector(0, 0, -31.0f));
		AnimInst->Montage_Play(SlideMontage);
		AnimInst->OnMontageEnded.AddDynamic(this, &AMainPlayer::EndSliding);
	}
}

// -31 <-무슨 값이더라
void AMainPlayer::EndSliding(UAnimMontage* Montage, bool bInterrupted)
{
	GetMesh()->SetRelativeLocation(FVector(0, 0, -90.0f));
	GetCapsuleComponent()->SetCapsuleHalfHeight(88);
	IsSliding = false;
}

void AMainPlayer::UseItem(FItemBaseData& Item)
{
	UE_LOG(LogTemp, Warning, TEXT("USE ITEM EXECUTED"));
	
	if (InventoryComponent)
	{
		FItemData* ItemData = InventoryComponent->GetItemData(Item.ItemID);
		
		if (ItemData->IsNotEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("TRY TO USE THIS : [ %s ]"), *ItemData->TextData.Name.ToString());
			
			if (StatusComponent && ItemData->Type == EItemType::FOOD)
			{
				if (ItemData->Type == EItemType::FOOD && EattingSound)
				{
					UGameplayStatics::PlaySound2D(GetWorld(), EattingSound);
				}
				StatusComponent->ApplyItem(*ItemData);
				//TODO: 서버 호출 함수로 벼경
				//InventoryComponent->
				//RemoveAmountOfItem(Item, 1);
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
	FItemBaseData* TargetItem = &InventoryComponent->GetItemAtIndex(HotBarIndex);
	
	if (true)
	{
		//사용 가능한 아이템이 아니면 암것두 안하긔.
		if (!InventoryComponent->IsUsableItem(*TargetItem)) return;
		
		UE_LOG(LogTemp, Warning, TEXT("ITEM IS VALID AND START USE ITEM"));
		
		if (!GetWorld()->GetTimerManager().IsTimerActive(ItemUseTimer))
		{
			FItemData* ItemData = InventoryComponent->GetItemData(*TargetItem);
			
			UE_LOG(LogTemp, Warning, TEXT("TIMER EXECUTED"));
			UE_LOG(LogTemp, Warning, TEXT("TARGET ITEM : [ %s ] : DURATION : [ %f ]"), *ItemData->TextData.Name.ToString(), ItemData->NumericData.InteractionDuration);
			
			
			GetWorld()->GetTimerManager().SetTimer(
			ItemUseTimer,
			[this, TargetItem]()
			{
				UseItem(*TargetItem);
			},
			1,
			false
			);
		}else
		{
			UE_LOG(LogTemp, Warning, TEXT("TIMER NOT EXECUTED"));
		}
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("NO ITEM IN HOTBAR SLOT [%d]"), HotBarIndex+1);
	}
}

void AMainPlayer::StopUseItem()
{
	UE_LOG(LogTemp, Warning, TEXT("USE ITEM TIMER CANCELED"));
	GetWorld()->GetTimerManager().ClearTimer(ItemUseTimer);
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
					if (Key == EKeys::One)      HotBarIndex = 0;
					else if (Key == EKeys::Two) HotBarIndex = 1;
					else if (Key == EKeys::Three) HotBarIndex = 2;
					else if (Key == EKeys::Four) HotBarIndex = 3;
					else if (Key == EKeys::Five) HotBarIndex = 4;
					else if (Key == EKeys::Six)  HotBarIndex = 5;

					HUD->RefreshHotBar();
					RefreshHand();
				}
			}
		}
	}
}

void AMainPlayer::HandleHotBarWithWheel(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Wheel Value : %f"), Value.Get<float>());
	if (Value.Get<float>() > 0)
	{
		HotBarIndex = (HotBarIndex + 1) % 6;
	} else
	{
		HotBarIndex = (HotBarIndex - 1 + 6) % 6;
	}

	HUD->RefreshHotBar();
	RefreshHand();
}

void AMainPlayer::OnDeath_Implementation()
{
	UE_LOG(LogTemp, Display, TEXT("Player Dead"));
}

//손에 든 아이템 업데이트 함수
void AMainPlayer::RefreshHand()
{
	if (!HasAuthority())
	{
		Server_RefreshHand();
	} else
	{
		//Multi_RefreshHand();	
	}
}

void AMainPlayer::Attack_Implementation()
{
	if (!HasAuthority())
	{
		Server_Attack();
	} else
	{
		Multi_Atack();
	}
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
		//여기 인터랙션 UI 해제 코드 추가 예정
		TargetInteractionInterface->EndFocus();
		return;
	}
	
	//여기 인터랙션 UI 업데이트 코드 추가 예정

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

		//여기 인터랙션 UI 업데이트 코드 추가 예정

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
			//인터랙션 액터의 인터랙션 시작 함수 실행
			TargetInteractionInterface->BeginInteract();

			//즉시 인터랙션이 가능하면 (꾹 누르는 인터랙션이 아니면)
			if (TargetInteractionInterface->InteractableData.InteractionDuration == 0.0f)
			{
				//인터랙션 가능 상태인지 확인
				if (TargetInteractionInterface->InteractableData.CanInteract)
				{
					//인터랙션 실행
					Interaction();
				}
			}
			//꾹 누르는 인터랙션이면
			else
			{
				//인터랙션 실행 시간 만큼 대기 후 인터랙션 실행
				GetWorldTimerManager().SetTimer(InteractionTimer,
					this,
					&AMainPlayer::Interaction,
					TargetInteractionInterface->InteractableData.InteractionDuration,
					false);
			}
		}
	}
}

void AMainPlayer::EndInteract()
{
	IInteractionInterface::EndInteract();

	//인터랙션 타이머 클리어
	GetWorldTimerManager().ClearTimer(InteractionTimer);

	//인터랙션 액터가 유효한 지 체크
	if (IsValid(TargetInteractionInterface.GetObject()))
	{
		//인터랙션 액터의 인터랙션 종료 함수 실행
		TargetInteractionInterface->EndInteract();
	}
}


void AMainPlayer::Interaction()
{
	//인터랙션 타이머 클리어
	GetWorldTimerManager().ClearTimer(InteractionTimer);
	
	//인터랙션 액터가 유효한 지 체크
	if (IsValid(TargetInteractionInterface.GetObject()))
	{
		//인터랙션 액터가 인터랙션 가능한 상태이면
		if (TargetInteractionInterface->InteractableData.CanInteract)
		{
			//인터랙션 액터의 인터랙션 함수 실행
			TargetInteractionInterface->Interact(this);
		}
	}
}

void AMainPlayer::DropItem(FItemBaseData& ItemToDrop, const int32 AmountToDrop, bool IsWhole)
{
	//UInventoryComponent* OriginInventory = ItemToDrop->OwningInventory;
	
	if (ItemToDrop.IsValid())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		const FVector SpawnLocation(GetActorLocation() + (GetActorForwardVector() * 50.0f));
		const FTransform SpawnTransform(GetActorRotation(), SpawnLocation);
		
		const int32 RemovedAmount = InventoryComponent->RemoveAmountOfItem(ItemToDrop, AmountToDrop);
		
		APickup* Pickup = GetWorld()->SpawnActor<APickup>(APickup::StaticClass(), SpawnTransform, SpawnParams);
		
		Pickup->InitializeDrop(ItemToDrop, RemovedAmount);

		if (ItemGettingSound)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), ItemGettingSound);
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

void AMainPlayer::WeaponTrace()
{
	//기본적으론 주먹 위치
	FVector3d StartPos = GetMesh()->GetSocketLocation(FName("hand_r"));
	FVector3d EndPos = GetMesh()->GetSocketLocation(FName("hand_r"));

	//무기를 장착 시 무기별 공격 범위로 설정
	if (ItemMesh)
	{
		StartPos = ItemMesh->GetSocketLocation(FName("HitBoxStart"));
		EndPos = ItemMesh->GetSocketLocation(FName("HitBoxEnd"));
	}

	//트레이스 파라미터 설정
	ETraceTypeQuery TraceTypeQuery = ETraceTypeQuery();
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);
	FHitResult Hit;

	//스피어 트레이스 실행
	if (UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		StartPos,
		EndPos,
		5.0f,
		TraceTypeQuery,
		true,
		IgnoreActors,
		EDrawDebugTrace::ForDuration,
		Hit,
		true))
	{
		//맞은 액터
		AActor* HitActor = Hit.GetActor();
		FItemBaseData HoldingItem = GetHoldingItemReference();

		//기본 대미지
		float Damage = 1.0f;

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

void AMainPlayer::StartWeaponAttack()
{
	GetWorld()->GetTimerManager().SetTimer(
		WeaponAttackTimer,
		this,
		&AMainPlayer::WeaponTrace,
		GetWorld()->DeltaTimeSeconds,
		true);
}

void AMainPlayer::EndWeaponAttack()
{
	//공격 트레이스 종료
	GetWorld()->GetTimerManager().ClearTimer(WeaponAttackTimer);
	//맞은 액터 배열 비우기
	DamagedActors.Empty();
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

//멀티플레이어 코드

void AMainPlayer::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AMainPlayer::Server_DropItem_Implementation(UInventoryComponent* SourceInventory, int32 SourceIndex, int32 AmountToDrop)
{
	
}

void AMainPlayer::Server_ToggleCrouch_Implementation()
{
	Multi_ToggleCrouch();
}

void AMainPlayer::Multi_ToggleCrouch_Implementation()
{
	//웅크리는 중이면
	if (IsCrouching)
	{
		UnCrouch();
		GetCharacterMovement()->MaxWalkSpeed = 300.0f;
		IsCrouching = false;
	} else
	{
		Crouch();
		GetCharacterMovement()->MaxWalkSpeed = 150.0f;
		IsCrouching = true;
	}
}

void AMainPlayer::Server_Run_Implementation()
{
	Multi_Run();
}

void AMainPlayer::Multi_Run_Implementation()
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
	
			GetCharacterMovement()->MaxWalkSpeed = 600.0f;
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

void AMainPlayer::Server_StopRun_Implementation()
{
	Multi_StopRun();	
}

void AMainPlayer::Multi_StopRun_Implementation()
{
	//달리기 중일 때만 달리기 중지 시퀀스 작동
	if (IsRunning)
	{
		GetCharacterMovement()->MaxWalkSpeed = 300.0f;
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

void AMainPlayer::Server_Attack_Implementation()
{
	Multi_Atack();
}

void AMainPlayer::Multi_Atack_Implementation()
{
	WeaponComponent->UseWeapon();
}

void AMainPlayer::Server_RefreshHand_Implementation()
{
	//Multi_RefreshHand();
}

void AMainPlayer::Multi_RefreshHand_Implementation()
{
	FItemBaseData Item = InventoryComponent->GetItemAtIndex(HotBarIndex);
	
	//해당 인덱스 인벤토리 칸에 아이템이 있으면 그 아이템 들기.
	if (Item.IsValid())
	{
		FItemData* ItemData = InventoryComponent->GetItemData(Item);
		
		IsHoldingItem = true;
		ItemMesh->SetStaticMesh(ItemData->AssetData.Mesh);
		ItemMesh->AttachToComponent(
		GetMesh(),
		FAttachmentTransformRules::KeepRelativeTransform,
		TEXT("hand_r"));
		
		FTransform SocketTransform = ItemMesh->GetSocketTransform(TEXT("HandSocket"), RTS_Component);
		ItemMesh->SetRelativeTransform(SocketTransform.Inverse());

		WeaponComponent->CheckWeapon(Item);
		//ItemMesh->SetRelativeScale3D(FVector(0.1f, 0.1f, 0.1f));
	} else
	{
		WeaponComponent->CheckWeapon(FItemBaseData());
		IsHoldingItem = false;
		ItemMesh->SetStaticMesh(nullptr);
	}
}