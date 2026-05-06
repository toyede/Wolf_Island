#include "Actors/PrayerForewarning.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"

APrayerForewarning::APrayerForewarning()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	AreaIndicator = CreateDefaultSubobject<USphereComponent>(TEXT("AreaIndicator"));
	RootComponent = AreaIndicator;
	AreaIndicator->SetSphereRadius(150.f);
	AreaIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ForewarningEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ForewarningEffect"));
	ForewarningEffectComponent->SetupAttachment(RootComponent);
	ForewarningEffectComponent->bAutoActivate = false;
}

void APrayerForewarning::BeginPlay()
{
	Super::BeginPlay();

	if (ForewarningEffect && ForewarningEffectComponent)
	{
		ForewarningEffectComponent->SetAsset(ForewarningEffect);
		ForewarningEffectComponent->Activate();
	}

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			TimerHandle,
			this,
			&APrayerForewarning::OnTimerEnd,
			Duration,
			false);
	}
}

void APrayerForewarning::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	
	Super::EndPlay(EndPlayReason);
}

void APrayerForewarning::OnTimerEnd()
{
	OnForewarningComplete.Broadcast();
	Destroy();
}
