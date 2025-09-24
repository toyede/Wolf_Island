// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatusComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHPZero);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaZero);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHungerZero);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHydrationZero);

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

	//체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float CurrentHP = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxHP = 100.0f;

	//스태미나
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float CurrentStamina = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxStamina = 100.0f;

	//배고픔
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float CurrentHunger = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxHunger = 100.0f;

	//수분
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float CurrentHydration = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxHydration = 100.0f;

	//무게
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float CurrentWeight = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
	float MaxWeight = 100.0f;
	
	//무게에 따른 감소율 증가분
	float AmountMultiplier = 1.0f;
	

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

	//배고픔 감소 주기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float HungerRate = 0.1f;
	//배고픔 감소량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float HungerAmount = 0.005f;
	//달리기 중 배고픔 감소량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float HungerAmountWhileRunning = 0.033f;

	//수분 감소 주기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float HydrationRate = 0.1f;
	//수분 감소량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float HydrationAmount = 0.0033f;
	//달리기 중 수분 감소향
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float HydrationAmountWhileRunning = 0.033f;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

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

	//수분 실시간 감소 시작 정지 함수
	UFUNCTION(BlueprintCallable)
	void StartHydration();
	UFUNCTION(BlueprintCallable)
	void StopHydration();

	//디버그 함수
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Debug")
	void DebugGetStatus(float& Stamina, float& Hunger, float& Hydration, float& Weight);
};
