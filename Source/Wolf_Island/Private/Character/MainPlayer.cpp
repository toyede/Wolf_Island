// Fill out your copyright notice in the Description page of Project Settings.


#include "Wolf_Island/Public/Character/MainPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Components/StatusComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


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
	UE_LOG(LogTemp, Warning, TEXT("bUsePawnControlRotation: %s"),
	FirstPersonCamera->bUsePawnControlRotation ? TEXT("true") : TEXT("false"));

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
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// 이동
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMainPlayer::Move);
		
		// 시야
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMainPlayer::Look);

		//달리기
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &AMainPlayer::Run);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &AMainPlayer::StopRun);

		//웅크리기
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AMainPlayer::ToggleCrouch);
		
		//인칭 변환 테스트 키
		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Started, this, &AMainPlayer::SwitchCamera);
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

void AMainPlayer::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	UE_LOG(LogTemp, Warning, TEXT("LOOK X: %f, Y: %f"), LookAxisVector.X, LookAxisVector.Y);
	float sen = 1;

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X * sen);
		AddControllerPitchInput(LookAxisVector.Y * sen);
	}
}

void AMainPlayer::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	UE_LOG(LogTemp, Warning, TEXT("MOVE X: %f, Y: %f"), MovementVector.X, MovementVector.Y);

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
		StatusComponent->StartStamina();
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