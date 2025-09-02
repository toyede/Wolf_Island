// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "MainPlayer.generated.h"
struct FInputActionValue;

UCLASS()
class WOLF_ISLAND_API AMainPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMainPlayer();
	
	//컴포넌트
	UPROPERTY(EditAnywhere)
	class UCameraComponent* FirstPersonCamera;
	
	UPROPERTY(EditAnywhere)
	UCameraComponent* ThirdPersonCamera;
	
	UPROPERTY(EditAnywhere)
	class USpringArmComponent* SpringArm;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStatusComponent* StatusComponent;
	
	//입력 관련 변수
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* RunAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* CrouchAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* SlideAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* AttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* InteractAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* InventoryAction;

	//상태 관련 변수 (뛰는 중인지, ~~하는 중인지 등등)
	//뛰는 중인지
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="State")
	bool IsRunning = false;
	
	//웅크리는 중인지
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="State")
	bool IsCrouching = false;

	//슬라이딩 중인지
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="State")
	bool IsSliding = false;
	
	//1인칭 카메라인지
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="State")
	bool IsFirstPerson = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	float InteractionCheckDistance = 300.0f;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void NotifyControllerChanged() override;

	UFUNCTION()
	void Look(const FInputActionValue& Value);
	
	UFUNCTION()
	void Move(const FInputActionValue& Value);

	UFUNCTION()
	void Run();

	UFUNCTION()
	void StopRun();

	UFUNCTION()
	void ToggleCrouch();

	UFUNCTION()
	void ToggleInventory();

	UFUNCTION()
	void SwitchCamera();

	UFUNCTION()
	void CheckInteraction();
};


