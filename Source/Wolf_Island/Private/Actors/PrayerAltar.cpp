#include "Actors/PrayerAltar.h"
#include "Components/BoxComponent.h"
#include "Character/MainPlayer.h"
#include "NiagaraFunctionLibrary.h"

APrayerAltar::APrayerAltar()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	AltarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AltarMesh"));
	RootComponent = AltarMesh;

	AltarZone = CreateDefaultSubobject<UBoxComponent>(TEXT("AltarZone"));
	AltarZone->SetupAttachment(RootComponent);
	AltarZone->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	AltarZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AltarZone->SetCollisionResponseToAllChannels(ECR_Overlap);
}

void APrayerAltar::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		AltarZone->OnComponentBeginOverlap.AddDynamic(this, &APrayerAltar::OnOverlapBegin);
		AltarZone->OnComponentEndOverlap.AddDynamic(this, &APrayerAltar::OnOverlapEnd);
	}
}

void APrayerAltar::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AMainPlayer* Player = Cast<AMainPlayer>(OtherActor))
	{
		PlayersOnAltar.AddUnique(Player);
	}
}

void APrayerAltar::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AMainPlayer* Player = Cast<AMainPlayer>(OtherActor))
	{
		PlayersOnAltar.Remove(Player);
	}
}

void APrayerAltar::NotifyEmoteStarted(AMainPlayer* Player, FName EmoteID)
{
	if (!HasAuthority()) return;

	if (PlayersOnAltar.Contains(Player) && EmoteID == RequiredEmoteID)
	{
		if (SuccessEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SuccessEffect, GetActorLocation());
		}

		OnPrayerSuccess.Broadcast();
	}
}
