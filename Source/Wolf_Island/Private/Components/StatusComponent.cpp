// Fill out your copyright notice in the Description page of Project Settings.


#include "Wolf_Island/Public/Components/StatusComponent.h"

#include "Character/MainPlayer.h"
#include "Character/MainPlayerController.h"
#include "Components/InventoryComponent.h"
#include "Games/MainGameState.h"
#include "Item/ItemBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UStatusComponent::UStatusComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	// ...
}


// Called when the game starts
void UStatusComponent::BeginPlay()
{
	Super::BeginPlay();
	
	SetIsReplicated(true);
	
	if (GetOwner()->HasAuthority())
	{
		//스태미나 다 쓰면 15초 이동 불가
		OnStaminaZero.AddDynamic(this, &UStatusComponent::ForcedRest);
		//배고픔 0일 시
		OnHungerZero.AddDynamic(this, &UStatusComponent::StartHungerDeath);
		//수분 0일 시
		OnHydrationZero.AddDynamic(this, &UStatusComponent::StartHydrationDeath);
		//산소 0일 시
		OnAirZero.AddDynamic(this, &UStatusComponent::StartAirDeath);
	}
}

void UStatusComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UStatusComponent::DestroyComponent(bool bPromoteChildren)
{
	if (GetOwner()->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[STATUS] CLEAR ALL TIMERS"));
		ClearAllTimers();
	}
	
	Super::DestroyComponent(bPromoteChildren);
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
	CurrentHP = FMath::Clamp(CurrentHP+amount, 0.0f, MaxHP);

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
	CurrentHP = FMath::Clamp(CurrentHP-amount, 0.0f, MaxHP);
	
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
	CurrentStamina = FMath::Clamp(CurrentStamina+amount*AmountMultiplier, 0.0f, MaxStamina);
	
	//초과 방지
	if (CurrentStamina >= MaxStamina)
	{
		CurrentStamina = MaxStamina;
		StopRecoverStamina();
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
	CurrentStamina = FMath::Clamp(CurrentStamina-amount*AmountMultiplier, 0.0f, MaxStamina);
	
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
	CurrentHunger = FMath::Clamp(CurrentHunger+amount, 0.0f, MaxHunger);
	StopHungerDeath();
	
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
	CurrentHunger = FMath::Clamp(CurrentHunger-amount*AmountMultiplier, 0.0f, MaxHunger);
	
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
	CurrentHydration = FMath::Clamp(CurrentHydration+amount, 0.0f, MaxHydration);
	StopHydrationDeath();
	
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
	CurrentHydration = FMath::Clamp(CurrentHydration-amount*AmountMultiplier, 0.0f, MaxHydration);
	
	//음수 방지
	if (CurrentHydration <= 0)
	{
		CurrentHydration = 0;
		OnHydrationZero.Broadcast();
	}
}

void UStatusComponent::IncreaseAir(float amount)
{
	CurrentAir = FMath::Clamp(CurrentAir+amount, 0.0f, MaxAir);
	
	if (CurrentAir >= MaxAir)
	{
		CurrentAir = MaxAir;
		OnAirFull.Broadcast();
		StopRecoverAir();
	}
	
	//음수 방지
	if (CurrentAir <= 0)
	{
		CurrentAir = 0;
		OnAirZero.Broadcast();
	}
}

void UStatusComponent::DecreaseAir(float amount)
{
	CurrentAir = FMath::Clamp(CurrentAir-amount, 0.0f, MaxAir);
	
	//음수 방지
	if (CurrentAir <= 0)
	{
		CurrentAir = 0;
		OnAirZero.Broadcast();
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
	if (!GetWorld()->GetTimerManager().IsTimerActive(HungerTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(
		HungerTimer,
		[this]()
		{
			DecreaseHunger(HungerAmount);
		},
		HungerRate,
		true);
	}
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
	if (GetWorld()->GetTimerManager().IsTimerActive(HungerDeathTimer))
	{
		GetWorld()->GetTimerManager().ClearTimer(HungerDeathTimer);
		MaxStamina = TempMaxStamina;
		StartRecoverStamina();
	}
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
		false);
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
	if (!GetWorld()->GetTimerManager().IsTimerActive(ForcedRestTimer))
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
	
	RecoverStamina();
}

void UStatusComponent::StartInfection()
{
	if (IsInfected) return;

	IsInfected = true;
	OnInfectionStarted.Broadcast(this);
}

void UStatusComponent::StopInfection()
{
	GetWorld()->GetTimerManager().ClearTimer(InfectionTimer);
	IsInfected = false;
	CurrentInfectionRate = 0.0f;
	OnInfectionChanged.Broadcast();
}

void UStatusComponent::IncreaseInfection()
{
	IncreaseInfectionBy(InfectionIncrement);
}

void UStatusComponent::IncreaseInfectionBy(float Amount)
{
	float PrevRate = CurrentInfectionRate;
	CurrentInfectionRate = FMath::Clamp(CurrentInfectionRate + Amount, 0.0f, MaxInfection);

	if (CurrentInfectionRate != PrevRate)
	{
		OnInfectionChanged.Broadcast();
	}
}

void UStatusComponent::DecreaseInfection(float Amount)
{
	CurrentInfectionRate = FMath::Clamp(CurrentInfectionRate - Amount, 0.0f, MaxInfection);
	OnInfectionChanged.Broadcast();
}

void UStatusComponent::StartAir()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(AirTimer)) return;
	
	StopRecoverAir();
	
	GetWorld()->GetTimerManager().SetTimer(
		AirTimer,
		[this]()
		{
			DecreaseAir(AirAmount);
		},
		AirRate,
		true);
}

void UStatusComponent::StopAir()
{
	GetWorld()->GetTimerManager().ClearTimer(AirTimer);
}

void UStatusComponent::StartAirDeath()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(AirDeathTimer)) return;
	UE_LOG(LogTemp, Warning, TEXT("START AIR DEATH"));
	GetWorld()->GetTimerManager().SetTimer(
		AirDeathTimer,
		[this]()
		{
			DecreaseHP(SuffocatedDamage);
		},
		SuffocatedRate,
		true);
}

void UStatusComponent::StopAirDeath()
{
	GetWorld()->GetTimerManager().ClearTimer(AirDeathTimer);
}

void UStatusComponent::StartRecoverAir()
{
	StopAir();
	
	if (GetWorld()->GetTimerManager().IsTimerActive(AirRecoverTimer)) return;
	
	GetWorld()->GetTimerManager().SetTimer(
		AirRecoverTimer,
		[this]()
		{
			IncreaseAir(AirRecoverAmount);
		},
		AirRecoverRate,
		true);
}

void UStatusComponent::StopRecoverAir()
{
	GetWorld()->GetTimerManager().ClearTimer(AirRecoverTimer);
}

void UStatusComponent::ApplyItem(FItemData Item)
{
	if (Item.IsNotEmpty())
	{
		IncreaseHP(Item.NumericData.Health);
		IncreaseStamina(Item.NumericData.Stamina);
		IncreaseHunger(Item.NumericData.Hunger);
		IncreaseHydration(Item.NumericData.Hydration);

		if (Item.ID == FName(TEXT("FO102")))
		{
			StopInfection();
			UE_LOG(LogTemp, Warning, TEXT("Infection Stopped"));
		}
		
		AMainGameState* GS = Cast<AMainGameState>(GetWorld()->GetGameState());
		AMainPlayerController* PC = Cast<AMainPlayerController>(Cast<APawn>(GetOwner())->GetController());
		FChattingData Data = FChattingData(
			"SYSTEM",Item.TextData.Name.ToString()+" USED", EMessageType::NOTICE);
		if (!PC) UE_LOG(LogTemp, Warning, TEXT("UStatusComponent::ApplyItem: PC is NULL"));
		PC->AddChat(Data);
	}
}

void UStatusComponent::ClearAllTimers()
{
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	
	TimerManager.ClearTimer(StaminaTimer);
	TimerManager.ClearTimer(StaminaRecoverTimer);
	TimerManager.ClearTimer(HungerTimer);
	TimerManager.ClearTimer(HydrationTimer);
	TimerManager.ClearTimer(HungerDeathTimer);
	TimerManager.ClearTimer(HydrationDeathTimer);
	TimerManager.ClearTimer(ForcedRestTimer);
	TimerManager.ClearTimer(InfectionTimer);
	TimerManager.ClearTimer(AirDeathTimer);
	TimerManager.ClearTimer(AirRecoverTimer);
	TimerManager.ClearTimer(AirTimer);
}

void UStatusComponent::DebugGetStatus(float &HP, float& Stamina, float& Hunger, float& Hydration)
{
	HP = CurrentHP;
	Stamina = CurrentStamina;
	Hunger = CurrentHunger;
	Hydration = CurrentHydration;
}

FStatusSaveData UStatusComponent::SaveStatus()
{
	UE_LOG(LogTemp, Warning, TEXT("[STATUS] Save Status"));
	
	FStatusSaveData Data = FStatusSaveData();
	
	Data.CurrentHP = CurrentHP;
	Data.CurrentStamina = CurrentStamina;
	Data.CurrentHunger = CurrentHunger;
	Data.CurrentHydration = CurrentHydration;
	Data.CurrentAir = CurrentAir;
	Data.AmountMultiplier = AmountMultiplier;
	Data.CurrentInfection = CurrentInfectionRate;
	
	return Data;
}

void UStatusComponent::LoadStatus(const FStatusSaveData& SaveData)
{
	UE_LOG(LogTemp, Warning, TEXT("[STATUS] Load Status"));
	
	CurrentHP = SaveData.CurrentHP;
	CurrentStamina = SaveData.CurrentStamina;
	CurrentHunger = SaveData.CurrentHunger;
	CurrentHydration = SaveData.CurrentHydration;
	CurrentAir = SaveData.CurrentAir;
	AmountMultiplier = SaveData.AmountMultiplier;
	CurrentInfectionRate = SaveData.CurrentInfection;
}

void UStatusComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStatusComponent, CurrentHP);
	DOREPLIFETIME(UStatusComponent, CurrentStamina);
	DOREPLIFETIME(UStatusComponent, CurrentHunger);
	DOREPLIFETIME(UStatusComponent, CurrentHydration);
	DOREPLIFETIME(UStatusComponent, CurrentAir);
	DOREPLIFETIME(UStatusComponent, CurrentInfectionRate);
	DOREPLIFETIME(UStatusComponent, IsInfected);
}

void UStatusComponent::OnRep_CurrentHunger()
{
	//UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentHunger : %f"), CurrentHunger);
}

float UStatusComponent::CalculateFinalDamage(AActor* Attacker, AActor* Target, float DamageAmount)
{
	UStatusComponent* AttackerStatus = Attacker->FindComponentByClass<UStatusComponent>();
	UStatusComponent* TargetStatus = Target->FindComponentByClass<UStatusComponent>();

	float Damage = DamageAmount;
	float TotalArmorPenetration = 0.0f;
	if (AttackerStatus != nullptr) 
	{
		Damage += AttackerStatus->Attack;
		TotalArmorPenetration += AttackerStatus->ArmorPenetration;
	}

	float TargetDefense = 0.0f;
	if (TargetStatus != nullptr) 
	{
		TargetDefense = TargetStatus->Armor; 
	}
	
	float FinalArmor = TargetDefense * ((100.0f - TotalArmorPenetration) / 100);
	float FinalDamage = Damage * ((100.0f - FinalArmor) / 100);

	FinalDamage = FMath::Max(0.0f, FinalDamage);
	FinalDamage = FMath::RoundToFloat(FinalDamage * 10.0f) / 10.0f;
	
	return FinalDamage;
}

float UStatusComponent::CalculateTrueDamage(AActor* Attacker, AActor* Target, float DamageAmount)
{
	UStatusComponent* AttackerStatus = Attacker->FindComponentByClass<UStatusComponent>();
	UStatusComponent* TargetStatus = Target->FindComponentByClass<UStatusComponent>();

	float TrueDamage = DamageAmount;
	if (AttackerStatus != nullptr) 
	{
		TrueDamage += AttackerStatus->Attack;
	}

	TrueDamage = FMath::Max(0.0f, TrueDamage);
	TrueDamage = FMath::RoundToFloat(TrueDamage * 10.0f) / 10.0f;
	
	return TrueDamage;
}
