#include "Actors/StatueForewarning.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "Character/MainPlayer.h"
#include "TimerManager.h"
#include "Engine/DamageEvents.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h" //KSH-경고 사운드 재생용

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

		//KSH-이펙트 크기를 실제 즉사 반경에 맞춰 표시 (경고 범위와 판정 범위 불일치 방지)
		if (bMatchEffectScaleToKillZone && KillZone && EffectBaseRadius > 0.0f)
		{
			const float Scale = KillZone->GetScaledSphereRadius() / EffectBaseRadius;
			ForewarningEffectComponent->SetWorldScale3D(FVector(Scale));
		}

		ForewarningEffectComponent->Activate();
	}
	else
	{
		//KSH-이펙트가 지정되지 않으면 플레이어에게 경고가 전혀 보이지 않는다
		UE_LOG(LogTemp, Error, TEXT("[StatueForewarning] ForewarningEffect is NOT set - player gets no visual warning!"));
	}

	//KSH-경고 사운드 재생
	if (ForewarningSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ForewarningSound, GetActorLocation());
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
