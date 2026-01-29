// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatusComponent.generated.h"

class UItemBase;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHPZero);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaZero);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHungerZero);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHydrationZero);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInfectionChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAirZero);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAirFull);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WOLF_ISLAND_API UStatusComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStatusComponent();

	//델리게이트
	UPROPERTY(BlueprintAssignable, Category="Status Delegate")
	FOnHPZero OnHPZero;
	UPROPERTY(BlueprintAssignable, Category="Status Delegate")
	FOnStaminaZero OnStaminaZero;
	UPROPERTY(BlueprintAssignable, Category="Status Delegate")
	FOnHungerZero OnHungerZero;
	UPROPERTY(BlueprintAssignable, Category="Status Delegate")
	FOnHydrationZero OnHydrationZero;
	UPROPERTY(BlueprintAssignable, Category="Status Delegate")
	FOnInfectionChanged OnInfectionChanged;
	UPROPERTY(BlueprintAssignable, Category="Status Delegate")
	FOnAirZero OnAirZero;
	UPROPERTY(BlueprintAssignable, Category="Status Delegate")
	FOnAirFull OnAirFull;
	

	//늑대인간
	//현재 감염률
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Infection")
	float CurrentInfectionRate = 0.0f;
	//감염 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Infection")
	bool IsInfected = false;
	//감염 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Infection")
	float InfectionInterval = 0.01f;
	//감염 증가율
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Infection")
	float InfectionIncrement = 0.01f;

	//체력
	//현재 체력
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Status")
	float CurrentHP = 100.0f;
	//최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxHP = 100.0f;

	//스태미나
	//현재 스태미나
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Status")
	float CurrentStamina = 100.0f;
	//최대 스태미나
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxStamina = 100.0f;
	//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float DeadLineStamina = 5.0f;
	float TempMaxStamina = 0.0f;

	//배고픔
	//현재 배고픔
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHunger, VisibleAnywhere, BlueprintReadWrite, Category = "Status")
	float CurrentHunger = 100.0f;
	//최대 배고픔
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxHunger = 100.0f;

	//수분
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Status")
	float CurrentHydration = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxHydration = 100.0f;

	//무게
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Status")
	float CurrentWeight = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxWeight = 100.0f;
	
	//무게에 따른 감소율 증가분
	float AmountMultiplier = 1.0f;
	
	//산소
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Status")
	float CurrentAir = 100.0f;
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxAir = 100.0f;
	

	//타이머
	//스태미나 타이머
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	FTimerHandle StaminaTimer;
	//스태미나 회복 타이머
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	FTimerHandle StaminaRecoverTimer;
	//배고픔 타이머
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	FTimerHandle HungerTimer;
	//수분 타이머
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	FTimerHandle HydrationTimer;
	//달리기 타이머
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	FTimerHandle RunningTimer;
	//배고픔 사망 타이머
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	FTimerHandle HungerDeathTimer;
	//수분 사망 타이머
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	FTimerHandle HydrationDeathTimer;
	//강제 휴식 타이머
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	FTimerHandle ForcedRestTimer;
	//감염 타이머
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	FTimerHandle InfectionTimer;
	//산소 타이머
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	FTimerHandle AirTimer;
	//산소 회복 타이머
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	FTimerHandle AirRecoverTimer;
	//산소 사망 타이머
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	FTimerHandle AirDeathTimer;

	//스태미나 감소 주기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float StaminaDecreaseRate = 0.1f;
	//스태미나 감소량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float StaminaDecreaseAmount = 0.1f;
	//스태미나 회복 주기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float StaminaRecovereRate = 0.1f;
	//스태미나 회복량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float StaminaRecoverAmount = 1.0f;
	//스태미나 회복 대기 시간(초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float StaminaRecoverDelay = 5.0f;
	//스태미나 강제 휴식 시간(초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float ForcedRestTime = 15.0f;

	//배고픔 감소 주기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float HungerRate = 0.1f;
	//배고픔 감소량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float HungerAmount = 0.005f;
	//달리기 중 배고픔 감소량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float HungerAmountWhileRunning = 0.033f;
	//배고픔 사망 타이머 시간(초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float HungerDeathRate = 60.0f;

	//수분 감소 주기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float HydrationRate = 0.1f;
	//수분 감소량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float HydrationAmount = 0.0033f;
	//달리기 중 수분 감소향
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float HydrationAmountWhileRunning = 0.033f;
	//수분 사망 타이머 시간(초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float HydrationDeathRate = 30.0f;
	
	//산소 감소 주기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float AirRate = 0.1f;
	//산소 감소량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float AirAmount = 0.001f;
	//산소 회복량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float AirRecoverAmount = 0.01f;
	//산소 회복 주기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float AirRecoverRate = 0.01f;
	//산소 부족으로 받는 대미지량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float SuffocatedDamage = 10.0f;
	//산소 부족으로 받는 대미지 주기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float SuffocatedRate = 1.0f;

	// 테스트용
	UPROPERTY(BlueprintReadWrite, Category = "Status")
	bool bIsIncapacitated = false;  // 쓰러진 상태

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//------------------------------------//
	//체력, 스태미나, 배고픔, 수분, 무게 증감 함수
	UFUNCTION(BlueprintCallable)
	void IncreaseHP(float amount);

	UFUNCTION(BlueprintCallable)
	void DecreaseHP(float amount);
	
	UFUNCTION(BlueprintCallable)
	void IncreaseStamina(float amount);
	
	UFUNCTION(BlueprintCallable)
	void DecreaseStamina(float amount);

	UFUNCTION(BlueprintCallable)
	void IncreaseHunger(float amount);

	UFUNCTION(BlueprintCallable)
	void DecreaseHunger(float amount);

	UFUNCTION(BlueprintCallable)
	void IncreaseHydration(float amount);

	UFUNCTION(BlueprintCallable)
	void DecreaseHydration(float amount);

	UFUNCTION(BlueprintCallable)
	void IncreaseWeight(float amount);

	UFUNCTION(BlueprintCallable)
	void DecreaseWeight(float amount);
	
	UFUNCTION(BlueprintCallable)
	void IncreaseAir(float amount);
	
	UFUNCTION(BlueprintCallable)
	void DecreaseAir(float amount);
	
	//-----------------------------------//

	//스태미나 감소, 회복 시작 정지 함수
	UFUNCTION(BlueprintCallable)
	void StartStamina();
	UFUNCTION(BlueprintCallable)
	void StopStamina();
	UFUNCTION(BlueprintCallable)
	void RecoverStamina();
	UFUNCTION(BlueprintCallable)
	void StartRecoverStamina();
	UFUNCTION(BlueprintCallable)
	void StopRecoverStamina();
	
	//배고픔 실시간 감소 시작 정지 함수
	UFUNCTION(BlueprintCallable)
	void StartHunger();
	UFUNCTION(BlueprintCallable)
	void StopHunger();
	//배고픔 사망 타이머 작동 정지 함수
	UFUNCTION(BlueprintCallable)
	void StartHungerDeath();
	UFUNCTION(BlueprintCallable)
	void StopHungerDeath();

	//수분 실시간 감소 시작 정지 함수
	UFUNCTION(BlueprintCallable)
	void StartHydration();
	UFUNCTION(BlueprintCallable)
	void StopHydration();
	//수분 사망 타이머 작동 정지 함수
	UFUNCTION(BlueprintCallable)
	void StartHydrationDeath();
	UFUNCTION(BlueprintCallable)
	void StopHydrationDeath();

	//강제 휴식 함수
	UFUNCTION(BlueprintCallable)
	void ForcedRest();
	//입력 차단 함수
	UFUNCTION(BlueprintCallable)
	void DisableController();
	//입력 허용 함수
	UFUNCTION(BlueprintCallable)
	void EnableController();

	//감염 시작 함수
	UFUNCTION(BlueprintCallable)
	void StartInfection();
	//감염 중단 함수
	UFUNCTION(BlueprintCallable)
	void StopInfection();
	//감염률 증가 함수
	UFUNCTION(BlueprintCallable)
	void IncreaseInfection();
	//감염률 감소 함수
	UFUNCTION(BlueprintCallable)
	void DecreaseInfection(float Amount);
	
	//산소 시작 함수
	UFUNCTION(BlueprintCallable)
	void StartAir();
	UFUNCTION(BlueprintCallable)
	void StopAir();
	UFUNCTION(BlueprintCallable)
	void StartAirDeath();
	UFUNCTION(BlueprintCallable)
	void StopAirDeath();
	UFUNCTION(BlueprintCallable)
	void StartRecoverAir();
	UFUNCTION(BlueprintCallable)
	void StopRecoverAir();

	//모든 타이머 삭제
	UFUNCTION(BlueprintCallable)
	void ClearAllTimers();

	//아이템 사용 스탯 적용 함수
	UFUNCTION(BlueprintCallable)
	void ApplyItem(FItemData Item);

	//값 반환 함수
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetHPPercent() { return CurrentHP / MaxHP; };
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetStaminaPercent() { return CurrentStamina / MaxStamina; };
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetHungerPercent() { return CurrentHunger / MaxHunger; };
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetHydrationPercent() { return CurrentHydration / MaxHydration; };
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetWeightPercent() { return CurrentWeight / MaxWeight; };
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetAirPercent() { return CurrentAir / MaxAir; };

	//디버그 함수
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Debug")
	void DebugGetStatus(float &HP, float& Stamina, float& Hunger, float& Hydration, float& Weight);

	//멀티플레이
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_CurrentHunger();
};