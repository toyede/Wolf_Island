// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Interaction/InteractionInterface.h"
#include "Data/ItemDataStruct.h"
#include "Widgets/NickName.h"
#include "Engine/DataTable.h"
#include "MainPlayer.generated.h"

class UInventory;
class ATorch;
class UWidgetComponent;
class UWaterBodyComponent;
class APickup;
class UPlayerHUD;
class UBillboardComponent;
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

	// 폴리지 상호작용 변수
	UPROPERTY(BlueprintReadOnly)
	UInstancedStaticMeshComponent* CurrentFoliageComponent;

	UPROPERTY(BlueprintReadOnly)
	int32 FoliageInstanceIndex;

	// 물 상호작용 변수 추가
	UPROPERTY(BlueprintReadOnly)
	UPrimitiveComponent* CurrentWaterComponent;
};

USTRUCT(BlueprintType)
struct FFoliageReward
{
	GENERATED_USTRUCT_BODY()

	// 획득할 아이템 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	// 획득할 아이템 수량
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemAmount = 1;

	// 파괴 시 소환할 BP
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AActor> SpawnBP;
};

UENUM(BlueprintType)
enum class ESwimMode : uint8
{
	NONE UMETA(DisplayName = "NONE"),
	TREADING UMETA(DisplayName = "TREADING"),
	SURFACE_SWIMMING UMETA(DisplayName = "SURFACE_SWIMMING"),
	UNDERWATER_IDLE UMETA(DisplayName = "UNDERWATER_IDLE"),
	UNDERWATER_SWIMMING UMETA(DisplayName = "UNDERWATER_SWIMMING")
};

USTRUCT(BlueprintType)
struct FAttackTracePoint
{
	GENERATED_USTRUCT_BODY();
	
	UPrimitiveComponent* Source;
	FVector Prev;
	FVector Curr;
};

UENUM(BlueprintType)
enum class ECharacterRole : uint8
{
	NONE UMETA(DisplayName = "NONE"),
	CAPTAIN UMETA(DisplayName = "CAPTAIN"),
	CHEF UMETA(DisplayName = "CHEF"),
	MECHANIC UMETA(DisplayName = "MECHANIC"),
	SOLDIER UMETA(DisplayName = "SOLDIER"),
	
};

UCLASS()
class WOLF_ISLAND_API AMainPlayer : public ACharacter, public IInteractionInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMainPlayer();
	
	virtual void PossessedBy(AController* NewController) override;
	
	virtual void PawnClientRestart() override;
	
	virtual void Restart() override;
	
	virtual void OnRep_PlayerState() override;
	
	virtual void OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class AMainPlayerController* MainPlayerController;
	//HUD=============================================================================
	//UPROPERTY(EditAnywhere)
	//class AMainHUD* HUD;
	
	//컴포넌트=========================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UCameraComponent* FirstPersonCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStatusComponent* StatusComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UInventoryComponent* InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UWeaponComponent* WeaponComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UBuildingComponent* BuildingComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAudioComponent* EatingSoundPlayer;
	
	//횃불
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Torch")
	ATorch* Torch;
	UPROPERTY(EditDefaultsOnly, Category = "Torch")
	TSubclassOf<ATorch> TorchClass;
	// 토치 전용 애님 레이어 (무기 DT 밖에서 별도 관리)
	UPROPERTY(EditDefaultsOnly, Category = "Torch")
	TSubclassOf<UAnimInstance> TorchAnimLayerClass;
	
	//부력 컴포넌트 - 수영을 위한 것
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UBuoyancyComponent* BuoyancyComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBillboardComponent* WaterLevelCheckPoint;
	
	UPROPERTY(BlueprintReadWrite)
	UAudioComponent* WaterAmbience;
	
	UPROPERTY(BlueprintReadWrite)
	UWaterBodyComponent* EnteredWater;
	
	UPROPERTY(BlueprintReadWrite)
	FTimerHandle SwimCheckHandle;

	//손에 들 아이템
	UPROPERTY(ReplicatedUsing=OnRep_HandedItem, EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* ItemMesh;
	
	//이동 관련 변수====================================================================	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Movement")
	float MovementMultiplier = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float KnockOutSpeed = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float WalkSpeed = 300.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float RunSpeed = 750.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float CrouchSpeed = 150.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float DefaultHeight = 88.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float DefaultZ = -88.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float CrouchHeight = 60.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float CrouchZ = -60.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Swim")
	float SwimmingSpeed = 300.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Swim")
	float SwimmingSprintSpeed = 500.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Swim")
	float WaterDeceleration = 0.4f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Swim")
	float WaterSurfaceOffset = 50.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement|Swim")
	float WaterSuffocatedOffest = 55.0f;
	
	//입력 관련 변수====================================================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* MoveAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* WaterElevationAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* RunAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* CrouchAction;
	
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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* DropItemAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Input")
	UInputAction* EmotionAction;

	//상태 관련 변수 (뛰는 중인지, ~~하는 중인지 등등)=====================================
	
	//캐릭터 역할 (선장, 요리사, 정비공, 군인)
	UPROPERTY(Replicated, BlueprintReadWrite, Category="State", SaveGame)
	ECharacterRole CharacterRole = ECharacterRole::NONE;
	
	//기절 타이머
	UPROPERTY(BlueprintReadWrite)
	FTimerHandle KnockOutTimer;
	
	//기절 후 사망까지 소요 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="State")
	float KnockOutToDeathTime = 30.0f;
	
	//뛰는 중인지
	UPROPERTY(ReplicatedUsing=OnRep_IsRunning, EditDefaultsOnly, BlueprintReadWrite, Category="State", SaveGame)
	bool IsRunning = false;

	//웅크리는 중인지
	UPROPERTY(ReplicatedUsing=OnRep_IsCrouching, EditDefaultsOnly, BlueprintReadWrite, Category="State", SaveGame)
	bool IsCrouching = false;
	
	//수영 중인지
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category="State", SaveGame)
	bool IsSwimming = false;
	
	//어떤 수영 인지-수면, 수중
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category="State", SaveGame)
	ESwimMode SwimMode = ESwimMode::NONE;
	
	//슬라이딩 중인지
	UPROPERTY(Replicated,EditDefaultsOnly, BlueprintReadWrite, Category="State", SaveGame)
	bool IsSliding = false;

	//1인칭 카메라인지
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="State", SaveGame)
	bool IsFirstPerson = true;

	//행동불능 상태인지
	UPROPERTY(ReplicatedUsing=OnRep_IsInability, EditDefaultsOnly, BlueprintReadWrite, Category="State", SaveGame)
	bool IsInability = false;

	//공격 소모 스태미나
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="State")
	float AttackConsumeAmount = 1.0f;

	//점프 소모 스태미나
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="State")
	float JumpConsumeAmount = 1.0f;

	//슬라이딩 소모 스태미나
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="State")
	float SlideConsumeAmount = 2.0f;

	//인벤토리가 열려 있는지
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category="State", SaveGame)
	bool IsInventoryOpen = false;

	//손에 든 아이템이 있는지
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadWrite, Category="State", SaveGame)
	bool IsHoldingItem = false;

	//공격 중인지
	UPROPERTY(Replicated,EditDefaultsOnly, BlueprintReadWrite, Category="State", SaveGame)
	bool IsAttacking = false;
	
	//공격 트레이스 실행 상태
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="State", SaveGame)
	bool IsTracingAttack = false;
	
	//아이템 사용 중인지
	UPROPERTY(Replicated,EditDefaultsOnly, BlueprintReadWrite, Category="State", SaveGame)
	bool IsUsingItem = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
	bool bBuildingInputBlocked = false;

	//핫바 관련 변수==================================================================
	//핫바 슬롯 인덱스
	UPROPERTY(ReplicatedUsing=OnRep_HotBarIndex, VisibleAnywhere, BlueprintReadOnly, Category="HotBar", SaveGame)
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
	UPROPERTY(ReplicatedUsing=OnRep_InteractionDuration, EditDefaultsOnly, BlueprintReadWrite, Category="Interaction")
	float InteractionDuration = 5.0f;
	//인터랙션 가능한 지
	UPROPERTY(ReplicatedUsing=OnRep_CanInteract, EditDefaultsOnly, BlueprintReadWrite, Category="Interaction", SaveGame)
	bool CanInteract = false;
	//날 인터랙트 하고 있는 다른 플레이어
	UPROPERTY(ReplicatedUsing=OnRep_Interacted)
	AMainPlayer* InteractingPlayer;

	//애니메이션 변수======================================================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Animations")
	class UAnimMontage* SlideMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Animations")
	UAnimMontage* PunchMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Animations")
	UAnimMontage* FuckyouMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Animations")
	UAnimMontage* PickUpMontage;
	// 스태미나 소진 주저앉기 몽타주 (일어서는 모션을 PlayRate -1로 역재생)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Animations")
	UAnimMontage* SitDownMontage;
	// 스태미나 회복 후 일어서기 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Animations")
	UAnimMontage* StandUpMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Animations")
	UAnimMontage* DrinkMontage;
	
	// 쓰러진 플레이어 살리는 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Animations")
	UAnimMontage* HealMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Animations")
	TArray<UAnimMontage*> EmotionMontages;

	//위젯=============================================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Widget")
	UPlayerHUD* HUD;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Widget")
	UWidgetComponent* NickName;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UNickName> NickNameWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Widget")
	UInventory* InventoryWidget;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Widget")
	TSubclassOf<UInventory>InventoryWidgetClass;

	//사운드============================================================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Sounds")
	class USoundBase* ItemGettingSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Sounds")
	USoundBase* JumpSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Sounds")
	USoundBase* DuringFoodSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Sounds")
	USoundBase* EatingSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Sounds")
	USoundBase* DrinkingSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Sounds")
	USoundBase* EwSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Sounds")
	USoundBase* PunchSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Sounds")
	USoundBase* UnderWaterAmbience;

	//공격===============================================================================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Attack")
	FTimerHandle WeaponAttackTimer;
	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> DamagedActors;
	
	//히트 포인트 소켓 이름 모음
	TArray<FName> HitSockets =
	{
		TEXT("HitPoint1"),
		TEXT("HitPoint2"),
		TEXT("HitPoint3"),
		TEXT("HitPoint4"),
		TEXT("HitPoint5"),
		TEXT("HitPoint6"),
		TEXT("HitPoint7"),
		TEXT("HitPoint8"),
		TEXT("HitPoint9"),
		TEXT("HitPoint10")
	};
	
	//트레이스 할 히트 포인트 모음
	TMap<FName, FAttackTracePoint> TracePoints;

	// 채집 시 플레이어 인벤토리로 들어올 아이템 정보 및 스폰될 BP가 담긴 맵
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TMap<UStaticMesh*, FFoliageReward> FoliageRewardMap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TSubclassOf<class AActor> OutlineActorClass;
	
	UPROPERTY(EditDefaultsOnly, Category="Interaction")
	TSubclassOf<APickup> ItemClass;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	AActor* CurrentOutlineActor;

public:
	// 폴리지를 찾았을 때 실행
	UFUNCTION()
	void FoundInteractableFoliage(UInstancedStaticMeshComponent* ISMC, int32 InstanceIndex);

	// 폴리지 상호작용
	UFUNCTION(Server, Reliable, BlueprintCallable, Category="Interaction")
	void Server_InteractFoliage(UInstancedStaticMeshComponent* ISMC, int32 InstanceIndex);

	//공격시 폴리지 판정
	UPROPERTY(EditAnywhere)
	TMap<UStaticMesh*, TSubclassOf<class ATree>> FoliageToActorMap;

	void TryConvertFoliageToActor(const FHitResult& HitResult, float DamageAmount);

	UFUNCTION(BlueprintCallable, Category = "Attack")
	void ProcessAttackHit(const FHitResult& HitResult, float DamageAmount);

	// 클라이언트에서 폴리지를 지우기 위한 멀티캐스트 함수
	UFUNCTION(NetMulticast, Reliable)
	void Multi_RemoveFoliageInstance(UInstancedStaticMeshComponent* ISMC, int32 InstanceIndex);
	
	//제작===================================================================================================
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting")
	UDataTable* RecipeDataTable;

	// 레시피 드롭다운 목록 생성 함수
	UFUNCTION(CallInEditor)
	TArray<FString> GetRecipeNames() const;

	// 시작 지급 레시피
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting", meta = (GetOptions = "GetRecipeNames"))
	TArray<FName> DefaultRecipes;
	
	// 이 플레이어가 특정 레시피를 제작할 수 있는지 (개인 + 공유) 종합 확인하는 함수
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crafting")
	bool HasRecipe(const FName& RecipeID) const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FTimerHandle CraftTimer;
	
	// 요리 및 수리 UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class URepairUI> RepairUIClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UBonFireUI> BonfireUIClass;
	
	// 요리 및 수리 UI 생성
	UFUNCTION(Client, Reliable, BlueprintCallable, Category="UI")
	void Client_OpenRepairUI(class ARepair_Actor* TargetActor);

	UFUNCTION(Client, Reliable, BlueprintCallable, Category="UI")
	void Client_OpenBonfireUI();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void Destroyed() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void NotifyControllerChanged() override;
	
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	//무게 업데이트 시 능력치 영향
	UFUNCTION()
	void OnCurrentWeightChanged();
	
	//컨트롤러에서 HUD 연결
	UFUNCTION()
	void SetHUDWidget(UPlayerHUD* NewHUD) { HUD = NewHUD; }

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
	UFUNCTION()
	void ToggleCrouch();

	//인벤토리 토글 함수
	UFUNCTION(BlueprintCallable)
	void ToggleInventory();

	//아이템 사용 함수
	UFUNCTION()
	void UseItem(int32 SlotIndex);

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
	
	//핫바 인덱스 변경 함수
	UFUNCTION()
	void SetHotbarIndex(int32 Index);

	//사망 시 함수
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void OnDeath();
	virtual void OnDeath_Implementation();

	//손 아이템 새로고침 함수
	UFUNCTION(BlueprintCallable)
	void RefreshHand();

	//공격 함수
	UFUNCTION()
	void Attack();
	
	//기절 함수
	UFUNCTION(BlueprintCallable)
	void KnockOut();
	
	//소생 함수
	UFUNCTION(BlueprintCallable)
	void Revive();
	
	//리스폰 처리 함수
	UFUNCTION(BlueprintCallable)
	void OnRespawn();
	
	//카메라 복구 함수
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void RestoreCamera();
	
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
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Interaction(AActor* Target);
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_InteractionExecuted();

	UFUNCTION(BlueprintCallable)
	void BeginInteract() override;
	UFUNCTION(BlueprintCallable)
	void EndInteract() override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	//아이템 떨구기 함수
	UFUNCTION(BlueprintCallable)
	void DropItem(UInventoryComponent* SourceInventory, int32 SourceIndex, int32 AmountToDrop, bool IsWhole);

	//손에 든 아이템 레퍼런스 반환 함수
	//UFUNCTION(BlueprintPure)
	FItemBaseData GetHoldingItemReference();

	//손에 든 아이템 타입 반환 함수
	UFUNCTION(BlueprintPure)
	EItemType GetHoldingItemType();

	//공격 무기 히트 트레이스 함수
	UFUNCTION(BlueprintCallable)
	void WeaponTrace(const FVector& StartPos, const FVector& EndPos);
	//무기 공격 트레이스 시작 함수
	UFUNCTION(BlueprintCallable)
	void StartWeaponAttack();
	//무기 공격 트레이스 종료 함수
	UFUNCTION(BlueprintCallable)
	void EndWeaponAttack();
	
	//손에 든 아이템 한 개 버리기
	UFUNCTION(BlueprintCallable)
	void DropItemOnHotBar();
	
	//수영 관련 함수=================================================================================
	//물 속 수직 움직임 입력
	UFUNCTION()
	void WaterElevation(const FInputActionValue& Value);
	
	//물에 진입 시 실행
	UFUNCTION(BlueprintCallable)
	void EnterWater(const FSphericalPontoon& Pontoon);
	
	//물에서 나올 시 실행
	UFUNCTION(BlueprintCallable)
	void ExitWater(const FSphericalPontoon& Pontoon);
	
	//산소바 숨기기
	UFUNCTION(BlueprintCallable)
	void HideAirBar();
	
	//수영 중 상태 체크-수면 위로 초과 이동 막기, 발이 땅에 닿는 수위면 수영 모드 종료 등등
	UFUNCTION(BlueprintCallable)
	void SwimCheck();
	
	//수영 모드 바꾸기
	UFUNCTION(BlueprintCallable)
	void SetSwimMode(ESwimMode NewSwimMode);
	
	//제작 관련 함수================================================================================
	UFUNCTION(BlueprintCallable)
	void StartCraft(FRecipeData RecipeData);
	
	UFUNCTION(BlueprintCallable)
	void StopCraft();

	//멀티플레이어==================================================================================
	//코드 리팩토링 방법 v1.0
	//실제 작동 함수, 서버 실행 함수(Server_XXX), 서버 요청 함수(Request_XXX)로 나눔
	//서버 요청 함수에서는 요청한 플레이어가 서버인지 클라이언트인지 확인하고
	//서버면 실제 작동 함수 실행, 클라이언트면 서버 실행 함수 실행
	//서버 실행 함수는 서버에 이 함수를 실행하겠다고 요청을 보냄.
	//서버 실행 함수 안에서는 실제 작동 함수를 실행시킴.
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	//감정표현
	UFUNCTION(BlueprintCallable)
	void Emotion(const FInputActionInstance& Instance);
	UFUNCTION()
	void Request_Emotion(UAnimMontage* Emotion);
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_Emotion(UAnimMontage* Emotion);
	
	UFUNCTION()
	void Request_StopEmotion(UAnimMontage* Emotion=nullptr);
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_StopEmotion(UAnimMontage* Emotion=nullptr);
	
	
	//인터랙션
	UFUNCTION()
	void OnRep_CanInteract() { InteractableData.CanInteract = CanInteract; };
	UFUNCTION()
	void OnRep_InteractionDuration() { InteractableData.InteractionDuration = InteractionDuration; };
	UFUNCTION()
	void OnRep_Interacted();
	
	UFUNCTION()
	void Request_BeginInteractPlayer(AActor* Target);
	UFUNCTION(Server, Reliable)
	void Server_BeginInteractPlayer(AActor* Target);
	UFUNCTION()
	void BeginInteractPlayer(AActor* Target);
	
	UFUNCTION()
	void Request_EndInteractPlayer(AActor* Target);
	UFUNCTION(Server, Reliable)
	void Server_EndInteractPlayer(AActor* Target);
	UFUNCTION()
	void EndInteractPlayer(AActor* Target);
		
	//달리기
	UFUNCTION()
	void Request_Run();
	UFUNCTION(Server, Reliable)
	void Server_Run();
	UFUNCTION()
	void Request_StopRun();
	UFUNCTION(Server, Reliable)
	void Server_StopRun();

	//웅크리기 
	UFUNCTION()
	void Request_ToggleCrouch();
	UFUNCTION(Server, Reliable)
	void Server_ToggleCrouch();
	UFUNCTION()
	void OnRep_IsCrouching();
	
	//기절
	UFUNCTION()
	void OnRep_IsInability();

	//공격
	UFUNCTION()
	void Request_Attack();
	UFUNCTION(Server, Reliable)
	void Server_Attack();
	UFUNCTION()
	void OnRep_IsRunning();

	//손에 든 아이템 새로고침
	UFUNCTION()
	void Request_RefreshHand();
	UFUNCTION(Server, Reliable)
	void Server_RefreshHand();
	
	//핫바 인덱스 수정
	UFUNCTION()
	void Request_SetHotbarIndex(int32 Index);
	UFUNCTION(Server, Reliable)
	void Server_SetHotbarIndex(int32 Index);
	UFUNCTION()
	void OnRep_HotBarIndex();
	
	//손에 든 아이템 메쉬 바뀌었을 때
	UFUNCTION()
	void OnRep_HandedItem();
	
	//아이템 드롭
	UFUNCTION()
	void Request_DropItem(UInventoryComponent* SourceInventory, int32 SourceIndex, int32 AmountToDrop, bool IsWhole);
	UFUNCTION(Server, Reliable)
	void Server_DropItem(UInventoryComponent* SourceInventory, int32 SourceIndex, int32 AmountToDrop, bool IsWhole);
	
	//아이템 사용
	UFUNCTION()
	void Request_StartUseItem();
	UFUNCTION(Server, Reliable)
	void Server_StartUseItem();
	UFUNCTION()
	void Request_StopUseItem();
	UFUNCTION(Server, Reliable)
	void Server_StopUseItem();
	
	//제작
	UFUNCTION()
	void Request_StartCraft(FRecipeData RecipeData);
	UFUNCTION(Server, Reliable)
	void Server_StartCraft(FRecipeData RecipeData);
	UFUNCTION()
	void Request_StopCraft();
	UFUNCTION(Server, Reliable)
	void Server_StopCraft();

	UFUNCTION(BlueprintCallable)
	void SetBuildingInputBlocked(bool bBlocked);

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsBuildingInputBlocked() const { return bBuildingInputBlocked; }
	
	//아이템 정보 저장
	
	//클라이언트 실행 함수 (UI 사운드 등 클라이언트 혼자만 보면 되는 것)
	UFUNCTION(Client, Reliable)
	void Client_PlaySound2D(USoundBase* Sound);
	
	UFUNCTION(Client, Reliable)
	void Client_ShowDeathScreen();
	
	UFUNCTION(Client, Reliable)
	void Client_BlinkEdge(FColor Color);
	
	UFUNCTION(Client, Reliable)
	void Client_HitEffect();
	
	//멀티캐스트 실행 함수
	UFUNCTION(NetMulticast, Reliable)
	void Multi_PlaySoundAtLocation(USoundBase* Sound, FVector Location);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multi_PlayAnimMontage(UAnimMontage* Anim);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multi_StopAnimMontage(UAnimMontage* Anim);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multi_PlayEatingSound();
	UFUNCTION(Client, Reliable)
	void Multi_StopEatingSound();

	// 강제 휴식 시작 - 주저앉기 몽타주 재생 (멀티캐스트)
	UFUNCTION(NetMulticast, Reliable)
	void Multi_PlayForcedRestStart();

	// 강제 휴식 종료 - 일어서기 몽타주 재생 (멀티캐스트)
	UFUNCTION(NetMulticast, Reliable)
	void Multi_PlayForcedRestEnd();

	// 주저앉기 몽타주 종료 콜백 - 마지막 프레임 유지용
	UFUNCTION()
	void OnSitDownMontageEnded(UAnimMontage* Montage, bool bInterrupted);


	// 물 마시기 추가(준행)
	
	// 물을 찾았을 때 실행
	UFUNCTION()
	void FoundInteractableWater(UPrimitiveComponent* WaterComp);

	// 물 마시기 서버 실행 함수
	UFUNCTION(Server, Reliable, BlueprintCallable, Category="Interaction")
	void Server_DrinkWater(UPrimitiveComponent* WaterComp);

	// 치유 몽타주 서버 실행 함수
	UFUNCTION(Server, Reliable)
	void Server_PlayHealMontage();

	UFUNCTION(Server, Reliable)
	void Server_StopHealMontage();

	// 물 마시기 쿨타임 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Interaction")
	float DrinkCooldown = 1.0f;

private:
	// 마지막으로 물을 마신 시간
	float LastDrinkTime = -999.0f;

protected:
	// 냄새 관련 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Scent")
	float ScentReportInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Scent")
	float ScentIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Scent")
	float ScentMaxRange = 1500.f;

	FTimerHandle ScentTimerHandle;

	// 실제 이벤트를 보내는 함수
	void ReportScent();
};
