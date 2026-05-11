#include "Actors/StatueForewarning.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "Character/MainPlayer.h"
#include "TimerManager.h"
#include "Engine/DamageEvents.h"
#include "DrawDebugHelpers.h"

AStatueForewarning::AStatueForewarning()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	KillZone = CreateDefaultSubobject<USphereComponent>(TEXT("KillZone"));
	RootComponent = KillZone;
	KillZone->SetSphereRadius(150.f);
	KillZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	KillZone->SetCollisionResponseToAllChannels(ECR_Overlap);

	ForewarningEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ForewarningEffect"));
	ForewarningEffectComponent->SetupAttachment(RootComponent);
	ForewarningEffectComponent->bAutoActivate = false;
}

void AStatueForewarning::BeginPlay()
{
	Super::BeginPlay();

	if (bDebugDrawWarning)
	{
		DrawDebugSphere(
			GetWorld(),
			GetActorLocation(),
			KillZone->GetScaledSphereRadius(),
			24,
			DebugSphereColor,
			false,
			ForewarningDuration,
			0,
			3.0f);
	}

	if (ForewarningEffect && ForewarningEffectComponent)
	{
		ForewarningEffectComponent->SetAsset(ForewarningEffect);
		ForewarningEffectComponent->Activate();
	}

	OnTelegraphStarted.Broadcast();
	BP_OnTelegraphStarted();

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			ForewarningTimerHandle,
			this,
			&AStatueForewarning::OnForewarningEnd,
			ForewarningDuration,
			false);
	}
}

void AStatueForewarning::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearAllTimersForObject(this);
	Super::EndPlay(EndPlayReason);
}

void AStatueForewarning::OnForewarningEnd()
{
	if (!HasAuthority())
	{
		return;
	}

	const int32 EliminatedPlayers = KillOverlappingPlayers();
	const bool bAreaClear = !HasOverlappingPlayers();

	OnTelegraphResolved.Broadcast(bAreaClear, EliminatedPlayers);
	BP_OnTelegraphResolved(bAreaClear, EliminatedPlayers);
	OnForewarningResolved.Broadcast(bAreaClear);
	OnForewarningComplete.Broadcast();

	Destroy();
}

int32 AStatueForewarning::KillOverlappingPlayers()
{
	TArray<AActor*> OverlappingActors;
	KillZone->GetOverlappingActors(OverlappingActors, AMainPlayer::StaticClass());

	int32 EliminatedCount = 0;
	for (AActor* Actor : OverlappingActors)
	{
		if (AMainPlayer* Player = Cast<AMainPlayer>(Actor))
		{
			Player->TakeDamage(LethalDamage, FDamageEvent(), nullptr, this);
			++EliminatedCount;
		}
	}

	return EliminatedCount;
}

bool AStatueForewarning::HasOverlappingPlayers() const
{
	TArray<AActor*> OverlappingActors;
	KillZone->GetOverlappingActors(OverlappingActors, AMainPlayer::StaticClass());
	return OverlappingActors.Num() > 0;
}
