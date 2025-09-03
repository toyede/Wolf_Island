// Fill out your copyright notice in the Description page of Project Settings.


#include "Wolf_Island/Public/Character/MainPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "MaterialHLSLTree.h"
#include "Components/StatusComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interaction/InteractionInterface.h"


// Sets default values
AMainPlayer::AMainPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StatusComponent = CreateDefaultSubobject<UStatusComponent>("StatusComponent");

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>("FirstPersonCamera");
	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>("ThirdPersonCamera");
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");

	GetMesh()->SetRelativeTransform(
		FTransform(
			FRotator(0, -90, 0),
			FVector(0,0,-90)
			));
	
	//메시에 카메라 붙이기
	FirstPersonCamera->SetupAttachment(GetMesh());
	//컨트롤러 마우스 위치 입력을 카메라 입력에 반영
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->SetRelativeTransform(
		FTransform(
			FRotator(0, 90, 0),
			FVector(0,20,170)
			));
	
	SpringArm->SetupAttachment(GetMesh());
	SpringArm->SetRelativeTransform(
		FTransform(
			FRotator(-20, 90, 0),
			FVector(0,0,150)
			));
	
	ThirdPersonCamera->SetupAttachment(SpringArm);

}

// Called when the game starts or when spawned
void AMainPlayer::BeginPlay()
{
	Super::BeginPlay();

	//상태 델리게이트 바인딩
	StatusComponent->OnStaminaZero.AddDynamic(this, &AMainPlayer::StopRun);

	StatusComponent->StartHunger();
	StatusComponent->StartHydration();
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
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMainPlayer::StartJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AMainPlayer::StopJumping);

		// 이동
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMainPlayer::Move);
		
		// 시야
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMainPlayer::Look);

		//달리기
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &AMainPlayer::Run);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &AMainPlayer::StopRun);

		//웅크리기
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AMainPlayer::ToggleCrouch);
		
		//인칭 변환 테스트 키 -- 폐기
		//EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Started, this, &AMainPlayer::SwitchCamera);

		//인터랙션
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AMainPlayer::BeginInteract);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &AMainPlayer::EndInteract);
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
	//달리는 중 점프하면 스태미나 감소 중단
	if (IsRunning)
	{
		StatusComponent->StopStamina();
	}

	//스태미나가 0이면 점프 불가
	if (StatusComponent->CurrentStamina <= JumpConsumeAmount)
	{
		return;
	}

	//낙하 중(점프 중) 이면 점프 불가
	if (GetCharacterMovement()->IsFalling())
	{
		return;
	}
	
	StatusComponent->DecreaseStamina(JumpConsumeAmount);
	
	Jump();
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
	//뛰는 중이 아니면
	if (!IsRunning)
	{
		//스태미나 0이면 암것도 안하기
		if (StatusComponent->CurrentStamina <= 0) return;
		
		GetCharacterMovement()->MaxWalkSpeed = 900.0f;
		StatusComponent->StopStamina();

		//이동 속도가 0 초과일 때만 스태미나 감소
		if (GetVelocity().Size() > 0)
		{
			StatusComponent->StartStamina();
		}
		
		IsRunning = true;
	}
}

void AMainPlayer::StopRun()
{	
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
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

void AMainPlayer::ToggleCrouch()
{
	//웅크리는 중이면
	if (IsCrouching)
	{
		UnCrouch();
		GetCharacterMovement()->MaxWalkSpeed = 600.0f;
		IsCrouching = false;
	} else
	{
		Crouch();
		GetCharacterMovement()->MaxWalkSpeed = 300.0f;
		IsCrouching = true;
	}
}

void AMainPlayer::ToggleInventory()
{
}

void AMainPlayer::SwitchCamera()
{
	if (IsFirstPerson)
	{
		FirstPersonCamera->SetActive(false);
		ThirdPersonCamera->SetActive(true);
		IsFirstPerson = false;
	} else
	{
		FirstPersonCamera->SetActive(true);
		ThirdPersonCamera->SetActive(false);
		IsFirstPerson = true;
	}
}

void AMainPlayer::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	if (IsRunning)
	{
		StatusComponent->StartStamina();
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
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 1.0f);
		
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
			if (FMath::IsNearlyZero(TargetInteractionInterface->InteractableData.InteractionDuration, 0.1f))
			{
				//인터랙션 가능 상태인지 확인
				if (TargetInteractionInterface->InteractableData.CanInteract)
				{
					//인터랙션 실행
					Interact();
				}
			}
			//꾹 누르는 인터랙션이면
			else
			{
				//인터랙션 실행 시간 만큼 대기 후 인터랙션 실행
				GetWorldTimerManager().SetTimer(InteractionTimer,
					this,
					&AMainPlayer::Interact,
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

void AMainPlayer::Interact()
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
