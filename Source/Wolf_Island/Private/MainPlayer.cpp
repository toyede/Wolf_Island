// Fill out your copyright notice in the Description page of Project Settings.


#include "Wolf_Island/Public/MainPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"


// Sets default values
AMainPlayer::AMainPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>("FirstPersonCamera");
	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>("ThirdPersonCamera");
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");

	//메시에 카메라 붙이기
	FirstPersonCamera->SetupAttachment(GetMesh());
	//컨트롤러 마우스 위치 입력을 카메라 입력에 반영
	FirstPersonCamera->bUsePawnControlRotation = true;
	
	SpringArm->SetupAttachment(GetMesh());
	ThirdPersonCamera->SetupAttachment(SpringArm);
	

}

// Called when the game starts or when spawned
void AMainPlayer::BeginPlay()
{
	Super::BeginPlay();
	
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

void AMainPlayer::Run()
{
}

void AMainPlayer::ToggleCrouch()
{
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