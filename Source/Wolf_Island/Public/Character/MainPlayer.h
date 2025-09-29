// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Interaction/InteractionInterface.h"
#include "MainPlayer.generated.h"
struct FInputActionValue;

USTRUCT(BlueprintType)
struct FInteractionData
{
	GENERATED_USTRUCT_BODY();

	FInteractionData() : CurrentInteractable(nullptr), LastInteractionCheckTime(0.0f)
	{

	};

	UPROPERTY(BlueprintReadOnly)
	AActor* CurrentInteractable;

	UPROPERTY(BlueprintReadOnly)
	float LastInteractionCheckTime;
};

UCLASS()
class WOLF_ISLAND_API AMainPlayer : public ACharacter , public IInteractionInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMainPlayer();
	
	//컴포넌트=========================================================================
	UPROPERTY(EditAnywhere)
	class UCameraComponent* FirstPersonCamera;
	
	UPROPERTY(EditAnywhere)
	UCameraComponent* ThirdPersonCamera;
	
	UPROPERTY(EditAnywhere)
	class USpringArmComponent* SpringArm;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStatusComponent* StatusComponent;
	
	//입력 관련 변수====================================================================
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

	//상태 관련 변수 (뛰는 중인지, ~~하는 중인지 등등)=====================================
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

	//공격 소모 스태미나
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="State")
	float AttackConsumeAmount = 1.0f;

	//점프 소모 스태미나
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="State")
	float JumpConsumeAmount = 1.0f;

	//슬라이딩 소모 스태미나
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="State")
	float SlideConsumeAmount  = 2.0f;

	//인터랙션 관련 변수===============================================================
	//인터랙션 타이머 - 꾹 누르는 인터랙션을 위한 것
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Interaction")
	FTimerHandle InteractionTimer;
	//인터랙션 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	float InteractionCheckDistance = 300.0f;
	//인터랙션 체크 빈도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	float InteractionCheckFrequency = 0.1f;
	//인터랙션 데이터 (인터랙션 액터, 인터랙션 시간)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction")
	FInteractionData InteractionData;
	//인터랙션 액터의 인터랙션 인터페이스 포인터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction")
	TScriptInterface<IInteractionInterface> TargetInteractionInterface;
	//꾹 누르기 인터랙션 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	float InteractionDuration = 0.0f;
	
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
	void StartJump();

	void Landed(const FHitResult& Hit) override;

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

	//인터랙션 관련 함수===================================================
	//인터랙션 체크 함수 - 라인트레이스로 인터랙션 액터 체크
	UFUNCTION()
	void CheckInteraction();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetInteractionDuration() { return InteractableData.InteractionDuration; };
	
	//인터랙션 액터를 찾았을 때
	UFUNCTION()
	void FoundInteractable(AActor* Interactable);
	
	//인터랙션 액터를 못 찾았을 때
	UFUNCTION()
	void NotFoundInteractable();
	
	//인터랙션 중인지 확인하는 함수==========================================
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsInteracting() const { return GetWorldTimerManager().IsTimerActive(InteractionTimer); };

	UFUNCTION(BlueprintCallable)
	void BeginInteract() override;
	UFUNCTION(BlueprintCallable)
	void EndInteract() override;
	UFUNCTION(BlueprintCallable)
	void Interaction();

	UFUNCTION(BlueprintImplementableEvent)
	void Interact(AActor* Interactor) override;
};


