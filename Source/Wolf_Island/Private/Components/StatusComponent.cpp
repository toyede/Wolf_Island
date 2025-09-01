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

void UStatusComponent::IncreaseHP(float amount)
{
	CurrentHP += amount;

	if (CurrentHP > MaxHP)
	{
		CurrentHP = MaxHP;
	}
}

void UStatusComponent::DecreaseHP(float amount)
{
	CurrentHP -= amount;

	if (CurrentHP <= 0)
	{
		CurrentHP = 0;
		OnHPZero.Broadcast();
	}
}

void UStatusComponent::IncreaseStamina(float amount)
{
	CurrentStamina += amount;

	if (CurrentStamina > MaxStamina)
	{
		CurrentStamina = MaxStamina;
	}
}

void UStatusComponent::DecreaseStamina(float amount)
{
	CurrentStamina -= amount;

	if (CurrentStamina <= 0)
	{
		CurrentStamina = 0;
		OnStaminaZero.Broadcast();
	}
}

void UStatusComponent::IncreaseHunger(float amount)
{
	CurrentHunger += amount;

	if (CurrentHunger > MaxHunger)
	{
		CurrentHunger = MaxHunger;
	}
}

void UStatusComponent::DecreaseHunger(float amount)
{
	CurrentHunger -= amount;

	if (CurrentHunger <= 0)
	{
		CurrentHunger = 0;
		OnHungerZero.Broadcast();
	}
}

void UStatusComponent::IncreaseHydration(float amount)
{
	CurrentHydration += amount;

	if (CurrentHydration > MaxHydration)
	{
		CurrentHydration = MaxHydration;
	}
}

void UStatusComponent::DecreaseHydration(float amount)
{
	CurrentHydration -= amount;

	if (CurrentHydration <= 0)
	{
		CurrentHydration = 0;
		OnHydrationZero.Broadcast();
	}
}

void UStatusComponent::StartStamina()
{
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

void UStatusComponent::StopStamina()
{
	GetWorld()->GetTimerManager().ClearTimer(StaminaTimer);
}

void UStatusComponent::RecoverStamina()
{
	GetWorld()->GetTimerManager().SetTimer(
		StaminaTimer,
		[this]()
		{
			IncreaseStamina(StaminaRecoverAmount);
		},
		StaminaDecreaseRate,
		true
	);
}

void UStatusComponent::StartRecoverStamina()
{
	GetWorld()->GetTimerManager().SetTimer(
		StaminaTimer,
		this,
		&UStatusComponent::RecoverStamina,
		StaminaRecoverDelay,
		false
	);
}

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

void UStatusComponent::StopHunger()
{
	GetWorld()->GetTimerManager().ClearTimer(HungerTimer);
}

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

void UStatusComponent::StopHydration()
{
	GetWorld()->GetTimerManager().ClearTimer(HydrationTimer);
}

