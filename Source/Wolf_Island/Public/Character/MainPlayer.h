// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Interaction/InteractionInterface.h"
#include "Data/ItemDataStruct.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	class UWeaponComponent* WeaponComponent;
	
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

	//공격 중인지
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="State")
	bool IsAttacking = false;

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
	
	//위젯=============================================================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Widget")
	TSubclassOf<class UPlayerHUD> HUDClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widget")
	UPlayerHUD* HUD;

	//사운드============================================================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Sounds")
	class USoundBase* ItemGettingSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Sounds")
	USoundBase* JumpSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Sounds")
	USoundBase* EattingSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Sounds")
	USoundBase* PunchSound;

	//공격===============================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attack")
	FTimerHandle WeaponAttackTimer;
	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> DamagedActors;
	
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

	//점프 시작 함수
	UFUNCTION()
	void StartJump();

	//착지 시 실행되는 함수
	void Landed(const FHitResult& Hit) override;

	//시야 조종 함수
	UFUNCTION()
	void Look(const FInputActionValue& Value);

	//이동 함수
	UFUNCTION()
	void Move(const FInputActionValue& Value);

	//달리기 시작 함수
	UFUNCTION()
	void Run();

	//달리기 중단 함수
	UFUNCTION()
	void StopRun();

	//웅크리기 토글 함수
	UFUNCTION(NetMulticast, Server, Reliable)
	void ToggleCrouch();

	//인벤토리 토글 함수
	UFUNCTION()
	void ToggleInventory();

	//슬라이딩 함수
	UFUNCTION()	
	void Sliding();

	//슬라이딩 끝났을 시 함수
	UFUNCTION()
	void EndSliding(UAnimMontage* Montage, bool bInterrupted);

	//아이템 사용 함수
	UFUNCTION()
	void UseItem(UItemBase* Item);

	//아이템 사용 시작 함수
	UFUNCTION()
	void StartUseItem();

	//아이템 사용 중단 함수
	UFUNCTION()
	void StopUseItem();

	//핫바 숫자키 선택 함수
	UFUNCTION()
	void HandleHotBar(const FInputActionValue& Value);

	//핫바 마우스 휠 선택 함수
	UFUNCTION()
	void HandleHotBarWithWheel(const FInputActionValue& Value);

	//사망 시 함수
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void OnDeath();
	virtual void OnDeath_Implementation();

	//손 아이템 새로고침 함수
	UFUNCTION(BlueprintCallable)
	void RefreshHand();

	//공격 함수
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
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

	//인터랙션 실행 함수
	UFUNCTION(BlueprintCallable)
	void Interaction();
	
	UFUNCTION(BlueprintCallable)
	void BeginInteract() override;
	UFUNCTION(BlueprintCallable)
	void EndInteract() override;
	UFUNCTION(BlueprintImplementableEvent)
	void Interact(AActor* Interactor) override;

	//아이템 떨구기 함수
	UFUNCTION(BlueprintCallable)
	void DropItem(UItemBase* ItemToDrop, const int32 AmountToDrop, bool IsWhole);

	//손에 든 아이템 레퍼런스 반환 함수
	UFUNCTION(BlueprintPure)
	UItemBase* GetHoldingItemReference();

	//손에 든 아이템 타입 반환 함수
	UFUNCTION(BlueprintPure)
	EItemType GetHoldingItemType();

	//공격 무기 히트 트레이스 함수
	UFUNCTION(BlueprintCallable)
	void WeaponTrace();
	//무기 공격 트레이스 시작 함수
	UFUNCTION(BlueprintCallable)
	void StartWeaponAttack();
	//무기 공격 트레이스 종료 함수
	UFUNCTION(BlueprintCallable)
	void EndWeaponAttack();
	
	//멀티플레이어==================================================================================
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

};


