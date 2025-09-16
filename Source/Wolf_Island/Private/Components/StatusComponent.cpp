// Fill out your copyright notice in the Description page of Project Settings.


#include "Wolf_Island/Public/Components/StatusComponent.h"

// Sets default values for this component's properties
UStatusComponent::UStatusComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UStatusComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void UStatusComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

//체력 증가 함수
void UStatusComponent::IncreaseHP(float amount)
{
	CurrentHP += amount;

	if (CurrentHP > MaxHP)
	{
		CurrentHP = MaxHP;
	}
}

//체력 감소 함수
void UStatusComponent::DecreaseHP(float amount)
{
	CurrentHP -= amount;

	if (CurrentHP <= 0)
	{
		CurrentHP = 0;
		OnHPZero.Broadcast();
	}
}

//스태미나 증가 함수
void UStatusComponent::IncreaseStamina(float amount)
{
	CurrentStamina += amount;

	if (CurrentStamina > MaxStamina)
	{
		CurrentStamina = MaxStamina;
	}
}

//스태미나 감소 함수
void UStatusComponent::DecreaseStamina(float amount)
{
	CurrentStamina -= amount;

	if (CurrentStamina <= 0)
	{
		CurrentStamina = 0;
		OnStaminaZero.Broadcast();
	}
}

//배고픔 증가 함수
void UStatusComponent::IncreaseHunger(float amount)
{
	CurrentHunger += amount;

	if (CurrentHunger > MaxHunger)
	{
		CurrentHunger = MaxHunger;
	}
}

//배고픔 감소 함수
void UStatusComponent::DecreaseHunger(float amount)
{
	CurrentHunger -= amount;

	if (CurrentHunger <= 0)
	{
		CurrentHunger = 0;
		OnHungerZero.Broadcast();
	}
}

//수분 증가 함수
void UStatusComponent::IncreaseHydration(float amount)
{
	CurrentHydration += amount;

	if (CurrentHydration > MaxHydration)
	{
		CurrentHydration = MaxHydration;
	}
}

//수분 감소 함수
void UStatusComponent::DecreaseHydration(float amount)
{
	CurrentHydration -= amount;

	if (CurrentHydration <= 0)
	{
		CurrentHydration = 0;
		OnHydrationZero.Broadcast();
	}
}

//스태미나 감소 시작 함수
//타이머를 등록하여 시작
void UStatusComponent::StartStamina()
{
	GetWorld()->GetTimerManager().ClearTimer(StaminaRecoverTimer);
	GetWorld()->GetTimerManager().SetTimer(
		StaminaTimer,
		[this]()
		{
			DecreaseStamina(StaminaDecreaseAmount);
		},
		StaminaDecreaseRate,
		true
	);
}

//스태미나 감소 중단 함수
//타이머를 클리어시켜 중단
void UStatusComponent::StopStamina()
{
	GetWorld()->GetTimerManager().ClearTimer(StaminaTimer);
}

//스태미나 회복 시작 함수
//타이머를 등록하여 시작
void UStatusComponent::RecoverStamina()
{
	GetWorld()->GetTimerManager().SetTimer(
		StaminaRecoverTimer,
		[this]()
		{
			IncreaseStamina(StaminaRecoverAmount);
		},
		StaminaDecreaseRate,
		true
	);
}

//스태미나 회복 시작 타이머 시작 함수
void UStatusComponent::StartRecoverStamina()
{
	GetWorld()->GetTimerManager().SetTimer(
		StaminaRecoverTimer,
		this,
		&UStatusComponent::RecoverStamina,
		StaminaRecoverDelay,
		false
	);
}

void UStatusComponent::StopRecoverStamina()
{
	GetWorld()->GetTimerManager().ClearTimer(StaminaRecoverTimer);
}

//배고픔 감소 시작 함수
void UStatusComponent::StartHunger()
{
	GetWorld()->GetTimerManager().SetTimer(
		HungerTimer,
		[this]()
		{
			DecreaseHunger(HungerAmount);
		},
		HungerRate,
		true
	);
}

//배고픔 감소 중단 함수
void UStatusComponent::StopHunger()
{
	GetWorld()->GetTimerManager().ClearTimer(HungerTimer);
}

//수분 감소 시작 함수
void UStatusComponent::StartHydration()
{
	GetWorld()->GetTimerManager().SetTimer(
		HydrationTimer,
		[this]()
		{
			DecreaseHydration(HydrationAmount);
		},
		HydrationRate,
		true
	);
}

//수분 감소 중단 함수
void UStatusComponent::StopHydration()
{
	GetWorld()->GetTimerManager().ClearTimer(HydrationTimer);
}

