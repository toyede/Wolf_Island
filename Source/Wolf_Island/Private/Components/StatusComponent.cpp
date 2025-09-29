// Fill out your copyright notice in the Description page of Project Settings.


#include "Wolf_Island/Public/Components/StatusComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

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

	//스태미나 다 쓰면 15초 이동 불가
	OnStaminaZero.AddDynamic(this, &UStatusComponent::ForcedRest);
	//배고픔 0일 시
	OnHungerZero.AddDynamic(this, &UStatusComponent::StartHungerDeath);
	//수분 0일 시
	OnHydrationZero.AddDynamic(this, &UStatusComponent::StartHydrationDeath);
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
	CurrentHP += FMath::Abs(amount);

	//초과 방지
	if (CurrentHP > MaxHP)
	{
		CurrentHP = MaxHP;
	}
	//음수 방지
	if (CurrentHP <= 0)
	{
		CurrentHP = 0;
		OnHPZero.Broadcast();
	}
}

//체력 감소 함수
void UStatusComponent::DecreaseHP(float amount)
{
	CurrentHP -= FMath::Abs(amount);

	//초과 방지
	if (CurrentHP > MaxHP)
	{
		CurrentHP = MaxHP;
	}
	//음수 방지
	if (CurrentHP <= 0)
	{
		CurrentHP = 0;
		OnHPZero.Broadcast();
	}
}

//스태미나 증가 함수
void UStatusComponent::IncreaseStamina(float amount)
{
	CurrentStamina += FMath::Abs(amount) * AmountMultiplier;
	
	//초과 방지
	if (CurrentStamina > MaxStamina)
	{
		CurrentStamina = MaxStamina;
	}
	//음수 방지
	if (CurrentStamina <= 0)
	{
		CurrentStamina = 0;
		OnStaminaZero.Broadcast();
	}
}

//스태미나 감소 함수
void UStatusComponent::DecreaseStamina(float amount)
{
	CurrentStamina -= FMath::Abs(amount) * AmountMultiplier;

	//초과 방지
	if (CurrentStamina > MaxStamina)
	{
		CurrentStamina = MaxStamina;
	}
	//음수 방지
	if (CurrentStamina <= 0)
	{
		CurrentStamina = 0;
		OnStaminaZero.Broadcast();
	}
}

//배고픔 증가 함수
void UStatusComponent::IncreaseHunger(float amount)
{
	CurrentHunger += FMath::Abs(amount);
	StopHungerDeath();

	//초과 방지
	if (CurrentHunger > MaxHunger)
	{
		CurrentHunger = MaxHunger;
	}
	//음수 방지
	if (CurrentHunger <= 0)
	{
		CurrentHunger = 0;
		OnHungerZero.Broadcast();
	}
}

//배고픔 감소 함수
void UStatusComponent::DecreaseHunger(float amount)
{
	CurrentHunger -= FMath::Abs(amount) * AmountMultiplier;

	//초과 방지
	if (CurrentHunger > MaxHunger)
	{
		CurrentHunger = MaxHunger;
	}
	//음수 방지
	if (CurrentHunger <= 0)
	{
		CurrentHunger = 0;
		OnHungerZero.Broadcast();
	}
}

//수분 증가 함수
void UStatusComponent::IncreaseHydration(float amount)
{
	CurrentHydration += FMath::Abs(amount);
	StopHydrationDeath();

	//초과 방지
	if (CurrentHydration > MaxHydration)
	{
		CurrentHydration = MaxHydration;
	}
	//음수 방지
	if (CurrentHydration <= 0)
	{
		CurrentHydration = 0;
		OnHydrationZero.Broadcast();
	}
}

//수분 감소 함수
void UStatusComponent::DecreaseHydration(float amount)
{
	CurrentHydration -= FMath::Abs(amount) * AmountMultiplier;

	//초과 방지
	if (CurrentHydration > MaxHydration)
	{
		CurrentHydration = MaxHydration;
	}
	//음수 방지
	if (CurrentHydration <= 0)
	{
		CurrentHydration = 0;
		OnHydrationZero.Broadcast();
	}
}

void UStatusComponent::IncreaseWeight(float amount)
{
	CurrentWeight += FMath::Abs(amount);

	//초과 방지
	if (CurrentWeight > MaxWeight)
	{
		CurrentWeight = MaxWeight;
	}
	//음수 방지
	if (CurrentWeight <= 0)
	{
		CurrentWeight = 0;
	}

	//무게에 따른 감소율 증가분 설정
	if (CurrentWeight == 100.0f)
	{
		AmountMultiplier = 1.5f;
	}
	else if (CurrentWeight >= 75.0f)
	{
		AmountMultiplier = 1.2f;
	}
	else if (CurrentWeight >= 50.0f)
	{
		AmountMultiplier = 1.1f;
	} else
	{
		AmountMultiplier = 1.0f;
	}
}

void UStatusComponent::DecreaseWeight(float amount)
{
	CurrentWeight -= FMath::Abs(amount);

	//초과 방지
	if (CurrentWeight > MaxWeight)
	{
		CurrentWeight = MaxWeight;
	}
	//음수 방지
	if (CurrentWeight <= 0)
	{
		CurrentWeight = 0;
	}

	//무게에 따른 감소율 증가분 설정
	if (CurrentWeight == 100.0f)
	{
		AmountMultiplier = 1.5f;
	}
	else if (CurrentWeight >= 75.0f)
	{
		AmountMultiplier = 1.2f;
	}
	else if (CurrentWeight >= 50.0f)
	{
		AmountMultiplier = 1.1f;
	} else
	{
		AmountMultiplier = 1.0f;
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

			DecreaseHydration(HydrationAmountWhileRunning);
			DecreaseHunger(HungerAmountWhileRunning);
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

//배고픔 0일 시 일정 시간 후 사망, 스태미나 5 고정
void UStatusComponent::StartHungerDeath()
{
	//이미 0이 되서 실행 중인 사망 타이머가 있으면 아무 것도 안함
	if (GetWorld()->GetTimerManager().IsTimerActive(HungerDeathTimer)) return;
	
	//스태미나 5로 고정
	TempMaxStamina = MaxStamina;
	MaxStamina = DeadLineStamina;
	CurrentStamina = MaxStamina;

	if (GetWorld()->GetTimerManager().IsTimerActive(StaminaRecoverTimer))
	{
		GetWorld()->GetTimerManager().ClearTimer(StaminaRecoverTimer);
	}
	
	GetWorld()->GetTimerManager().SetTimer(
		HungerDeathTimer,
		[this]()
		{
			DecreaseHP(MaxHP);
		},
		HungerDeathRate,
		false
		);
}

void UStatusComponent::StopHungerDeath()
{
	GetWorld()->GetTimerManager().ClearTimer(HungerDeathTimer);
	MaxStamina = TempMaxStamina;
	StartRecoverStamina();
}

//수분 0일 시 일정 시간 후 사망
void UStatusComponent::StartHydrationDeath()
{
	//이미 0이 되서 실행 중인 사망 타이머가 있으면 아무 것도 안함
	if (GetWorld()->GetTimerManager().IsTimerActive(HydrationDeathTimer)) return;
	
	GetWorld()->GetTimerManager().SetTimer(
		HydrationDeathTimer,
		[this]()
		{
			DecreaseHP(MaxHP);
		},
		HydrationDeathRate,
		false
		);
}

void UStatusComponent::StopHydrationDeath()
{
	GetWorld()->GetTimerManager().ClearTimer(HydrationDeathTimer);
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

void UStatusComponent::ForcedRest()
{
	DisableController();
	
	GetWorld()->GetTimerManager().SetTimer(
		ForcedRestTimer,
		this,
		&UStatusComponent::EnableController,
		ForcedRestTime,
		false
	);

}

void UStatusComponent::DisableController()
{
	APawn* Owner = Cast<APawn>(GetOwner());
	if (Owner)
	{
		APlayerController* Controller = Cast<APlayerController>(Owner->GetController());

		if (Controller)
		{
			Owner->DisableInput(Controller);
		}
	}
}

void UStatusComponent::EnableController()
{
	APawn* Owner = Cast<APawn>(GetOwner());
	if (Owner)
	{
		APlayerController* Controller = Cast<APlayerController>(Owner->GetController());

		if (Controller)
		{
			Owner->EnableInput(Controller);
		}
	}	
}

void UStatusComponent::DebugGetStatus(float& Stamina, float& Hunger, float& Hydration, float& Weight)
{
	Stamina = CurrentStamina;
	Hunger = CurrentHunger;
	Hydration = CurrentHydration;
	Weight = CurrentWeight;
}
