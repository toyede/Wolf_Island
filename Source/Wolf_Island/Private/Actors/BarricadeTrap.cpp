// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/BarricadeTrap.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABarricadeTrap::ABarricadeTrap()
{
	PrimaryActorTick.bCanEverTick = false; 
	bReplicates = true; 
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	DamageBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageBox"));
	DamageBox->SetupAttachment(RootComponent);
	CurrentAttackCount = 0;

}

// Called when the game starts or when spawned
void ABarricadeTrap::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && DamageBox)
	{
		DamageBox->OnComponentBeginOverlap.AddDynamic(this, &ABarricadeTrap::OnOverlapBegin);
		DamageBox->OnComponentEndOverlap.AddDynamic(this, &ABarricadeTrap::OnOverlapEnd);
	}
}

void ABarricadeTrap::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		OverlappingActors.Add(OtherActor);

		PerformAttack(OtherActor);

		if (!GetWorld()->GetTimerManager().IsTimerActive(DamageTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(DamageTimerHandle, this, &ABarricadeTrap::DealPeriodicDamage, DamageInterval, true);
		}
	}
}

void ABarricadeTrap::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		OverlappingActors.Remove(OtherActor);

		if (OverlappingActors.Num() == 0)
		{
			GetWorld()->GetTimerManager().ClearTimer(DamageTimerHandle);
		}
	}
}

void ABarricadeTrap::Multicast_PlayDestroyEffects_Implementation()
{
	if (DestroySound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DestroySound, GetActorLocation());
	}
}

void ABarricadeTrap::DealPeriodicDamage()
{
	TArray<AActor*> ActorsToDamage = OverlappingActors.Array();

	for (AActor* Actor : ActorsToDamage)
	{
		if (CurrentAttackCount >= MaxAttackCount)
		{
			break;
		}

		if (Actor)
		{
			PerformAttack(Actor);
		}
	}
}

void ABarricadeTrap::PerformAttack(AActor* TargetActor)
{
	if (TargetActor && CurrentAttackCount < MaxAttackCount)
	{
		UGameplayStatics::ApplyDamage(TargetActor, DamageAmount, nullptr, this, UDamageType::StaticClass());
		CurrentAttackCount++;

		if (CurrentAttackCount >= MaxAttackCount)
		{
			GetWorld()->GetTimerManager().ClearTimer(DamageTimerHandle);
			Multicast_PlayDestroyEffects();
			Destroy(); 
		}
	}
}


