// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AI/AIControllers/EnemyAIController.h"
#include "AI/Interfaces/EnemyCommonInterface.h"
#include "AI/Interfaces/AttackMeshProvider.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimMontage.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "EnemyAIBase.generated.h"

class APatrolRoute;
class UAnimMontage;
class UAttackCollisionComponent;
class UDamageType;
class APickup;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitResponse); // 맞을 때 피격 모션 바인딩용
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnThrowEnd); // 투척 공격 끝났음을 알리는 용도
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackEnd); // 기본 공격 끝났음을 알리는 용도
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHowlingEnd); // 하울링 끝났음을 알리는 용도
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHPChangeEnd); // 체력 변화 알림용

UENUM(BlueprintType) // 상태 구분
enum class EEnemyForm : uint8
{
	Human UMETA(DisplayName = "Human"),
	Wolf UMETA(DisplayName = "Wolf")
};

UCLASS()
class WOLF_ISLAND_API AEnemyAIBase : public ACharacter, public IEnemyCommonInterface, public IAttackMeshProvider
{
	GENERATED_BODY()

public:
	AEnemyAIBase();

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnHitResponse OnHitResponse;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnThrowEnd OnThrowEnd;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnAttackEnd OnAttackEnd;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnHowlingEnd OnHowlingEnd;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnHPChangeEnd OnHPChangeEnd;

protected:
	virtual void BeginPlay() override;
	
	virtual void OnRep_Controller() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// 하늘 가져와야 해!!!!!!!
	UFUNCTION(BlueprintCallable)
	void NotifySkyRemoveSelf();

	// 패트롤
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol Route")
	TObjectPtr<APatrolRoute> AssignedPatrolRoute; // 이걸 바로 할당하진 않음

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Patrol Route")
	TObjectPtr<APatrolRoute> NativePatrolRoute; // 원주민일 때 쓸 패트롤루트 

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Patrol Route")
	TObjectPtr<APatrolRoute> WolfPatrolRoute; // 늑대일 때 쓸 패트롤루트 

	UFUNCTION(BlueprintCallable)
	int32 GetNextPoint();

	UFUNCTION(BlueprintCallable)
	int32 GetRandomPointIndex();

	UPROPERTY(VisibleAnywhere)
	int32 CurrentPatrolIndex = 0;

	UPROPERTY()
	TObjectPtr<AActor> CachedSkyManager = nullptr;

	// 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Comp")
	class UStatusComponent* StatusComponent; 

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Comp")
	UAttackCollisionComponent* AttackCollisionComponent;

	//상태 변환 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_EnemyForm, Category = "Stats")
	EEnemyForm EnemyForm;

	UFUNCTION()
	void OnRep_EnemyForm();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerChangeForm(EEnemyForm Form);

	void ApplyFormVisuals();

	void ApplySpeedByState(EEnemyState State);

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimMontage* ChangeFormMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Form Change")
	bool bIsHuman = true;

	// 임시 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float AttackDamage;

	// 메시
	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USkeletalMeshComponent* FaceMesh;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USkeletalMeshComponent* TorsoMesh;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USkeletalMeshComponent* LegsMesh;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USkeletalMeshComponent* FeetMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* WolfMesh;

	UPROPERTY()
	TArray<USkeletalMeshComponent*> HumanParts;

	// 애님블프

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Form Change")
	TSubclassOf<UAnimInstance> HumanAnimBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Form Change")
	TSubclassOf<UAnimInstance> WolfAnimBP;

	// 이펙트
	UPROPERTY(EditAnywhere, Category = "Particle")
	class UNiagaraSystem* FormChangeNiagaraEffect;

	UFUNCTION()
	void SpawnParticle();

	//사운드
	UFUNCTION(BlueprintCallable, Category = "Sound")
	void Growling();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* GrowlSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundAttenuation* AISoundAttenuation;
	
	// 인터페이스
	virtual void SetMovementSpeed_Implementation(EEnemyState State) override;

	virtual void ThrowObject_Implementation() override;

	virtual void Die_Implementation() override;

	virtual void NormalAttack_Implementation() override;

	virtual void Howling_Implementation() override;

	// 행동
	
	UFUNCTION()
	void Heal();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float HealAmount = 20.f;

	UFUNCTION()
	void OnStateChanged(EEnemyState NewState);

	FTimerHandle HealTimer;

	UPROPERTY(EditDefaultsOnly)
	float HealInterval = 2.0f;

	// 몽타주 제어
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void StopAllMontages();

	UFUNCTION()
	void OnThrowMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnFrozenMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnHowlingMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// 타격
	UFUNCTION()
	void HitResponse();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HitResponse();

	// 컨트롤러
	UPROPERTY(BlueprintReadWrite, Category = "AI|Controller")
	TObjectPtr<AEnemyAIController> EnemyAIController;

	// 이동속도
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Movement")
	float PassiveSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Form")
	float NativePatrolSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Form")
	float WolfPatrolSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AttackingSpeed = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DeadSpeed = 0.f;

	// 전투 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|ThrowObject")
	TSubclassOf<AActor> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|ThrowObject")
	TObjectPtr<UAnimMontage> ThrowMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|ThrowObject")
	TObjectPtr<USoundBase> ThrowSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|ThrowObject")
	FName ThrowSocketName = TEXT("hand_r");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Frozen")
	TObjectPtr<UAnimMontage> FrozenMontage_Native;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Frozen")
	TObjectPtr<UAnimMontage> FrozenMontage_Wolf;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Frozen")
	TObjectPtr<USoundBase> FrozenHitSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Dead", ReplicatedUsing = OnRep_IsDead)
	bool bIsDead = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Dead")
	TObjectPtr<USoundBase> DieSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	TObjectPtr<USoundBase> AttackSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	TSubclassOf<UDamageType> WolfAttackDamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Howling")
	TObjectPtr<UAnimMontage> HowlingMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Howling")
	TObjectPtr<USoundBase> HowlingSound;


	UFUNCTION()
	void OnAttackHit(const FHitResult& HitResult);

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayThrowMontage();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayNormalAttackMontage();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayHowlingMontage();

	UFUNCTION()
	void OnRep_IsDead();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void ApplyDeadState();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dead|DropItem")
	TSubclassOf<APickup> DropItemClass;

	UPROPERTY(EditDefaultsOnly, Category = "Dead|DropItem")
	FDataTableRowHandle DropItemHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Dead|DropItem")
	int32 MinDropAmount = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Dead|DropItem")
	int32 MaxDropAmount = 1;

	void DropItem();

public:
	virtual USkeletalMeshComponent* GetAttackMesh() const override
	{
		return bIsHuman ? GetMesh() : WolfMesh;
	}

	virtual UAttackCollisionComponent* GetAttackCollisionComponent() const override
	{
		return AttackCollisionComponent;
	}
};
