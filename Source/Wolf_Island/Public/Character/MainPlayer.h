// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Interaction/InteractionInterface.h"
#include "MainPlayer.generated.h"

class UItemBase;
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

	//HUD=============================================================================
	//UPROPERTY(EditAnywhere)
	//class AMainHUD* HUD;
	
	//컴포넌트=========================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	class UCameraComponent* FirstPersonCamera;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	class UStatusComponent* StatusComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	class UInventoryComponent* InventoryComponent;
	//손에 들 아이템
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	UStaticMeshComponent* ItemMesh;
	
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* UseItemAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* HotBarAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* HotBarWheelAction;

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

	//행동불능 상태인지
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="State")
	bool IsInability = false;

	//공격 소모 스태미나
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="State")
	float AttackConsumeAmount = 1.0f;

	//점프 소모 스태미나
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="State")
	float JumpConsumeAmount = 1.0f;

	//슬라이딩 소모 스태미나
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="State")
	float SlideConsumeAmount  = 2.0f;

	//인벤토리가 열려 있는지
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="State")
	bool IsInventoryOpen = false;

	//손에 든 아이템이 있는지
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="State")
	bool IsHoldingItem = false;

	//핫바 관련 변수==================================================================
	//핫바 슬롯 인덱스
	UPROPERTY(VisibleAnywhere, Category="HotBar")
	int32 HotBarIndex = 0;
	UPROPERTY(VisibleAnywhere, Category="HotBar")
	FTimerHandle ItemUseTimer;

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

	//애니메이션 변수======================================================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Animations")
	class UAnimMontage* SlideMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Animations")
	UAnimMontage* PunchMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Animations")
	UAnimMontage* FuckyouMontage;
	
	//위젯============================================================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Widget")
	TSubclassOf<class UPlayerHUD> HUDClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widget")
	UPlayerHUD* HUD;

	//공격시 폴리지 판정
	UPROPERTY(EditAnywhere, Category = "Interaction")
	TMap<UStaticMesh*, TSubclassOf<class ATree>> FoliageToActorMap;

	void TryConvertFoliageToActor(const FHitResult& HitResult, float DamageAmount);
	
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ProcessAttackHit(const FHitResult& HitResult, float DamageAmount);
	
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

	UFUNCTION(NetMulticast, Server, Reliable)
	void ToggleCrouch();

	UFUNCTION()
	void ToggleInventory();

	UFUNCTION()
	void Sliding();

	UFUNCTION()
	void EndSliding(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void UseItem(UItemBase* Item);

	UFUNCTION()
	void StartUseItem();

	UFUNCTION()
	void StopUseItem();

	UFUNCTION()
	void HandleHotBar(const FInputActionValue& Value);

	UFUNCTION()
	void HandleHotBarWithWheel(const FInputActionValue& Value);
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void OnDeath();
	virtual void OnDeath_Implementation();

	UFUNCTION(BlueprintCallable)
	void RefreshHand();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void Attack();
	virtual void Attack_Implementation();

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

	UFUNCTION(BlueprintCallable)
	void DropItem(UItemBase* ItemToDrop, const int32 AmountToDrop, bool IsWhole);

	UFUNCTION(BlueprintPure)
	UItemBase* GetHoldingItemReference();
	
	//멀티플레이어
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

};


