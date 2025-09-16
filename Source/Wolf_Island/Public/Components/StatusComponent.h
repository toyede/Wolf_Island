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

	//스태미나 배고픔 수분 감소율
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float StaminaDecreaseRate = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float StaminaDecreaseAmount = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float StaminaRecovereRate = 0.05f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float StaminaRecoverAmount = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float StaminaRecoverDelay = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float HungerRate = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float HungerAmount = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float HydrationRate = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Setting")
	float HydrationAmount = 0.1f;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//------------------------------------//
	//체력, 스태미나, 배고픔, 수분 증감 함수
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
};
