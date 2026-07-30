// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/HitParticleComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UHitParticleComponent::UHitParticleComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);	
	// ...
}


// Called when the game starts
void UHitParticleComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakePointDamage.AddDynamic(this, &UHitParticleComponent::HandlePointDamage);
	}
}

void UHitParticleComponent::Multi_PlayHitParticle_Implementation(FHitResult Hit)
{
	PlayHitParticleAt(Hit);
}

// Called every frame
void UHitParticleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHitParticleComponent::HandlePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy,
	FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection,
	const UDamageType* DamageType, AActor* DamageCauser)
{
	FHitResult Hit;
	Hit.Location    = HitLocation;
	Hit.ImpactPoint  = HitLocation;
	Hit.ImpactNormal = -ShotFromDirection;
	Hit.BoneName     = BoneName;
	Hit.Component    = FHitComponent;

	if (FHitComponent)
	{
		if (FBodyInstance* Body = FHitComponent->GetBodyInstance(BoneName))
		{
			Hit.PhysMaterial = Body->GetSimplePhysicalMaterial();
		}
	}

	Multi_PlayHitParticle(Hit);
}

void UHitParticleComponent::PlayHitParticleAt(const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("PlayHitParticleAt"));
	UNiagaraSystem* ParticleToPlay = DefaultHitParticle;

	if (Hit.PhysMaterial.IsValid())
	{
		EPhysicalSurface Surface = UGameplayStatics::GetSurfaceType(Hit);
		if (UNiagaraSystem** Found = SurfaceHitParticles.Find(Surface))
		{
			ParticleToPlay = *Found;
		}
	}

	if (!ParticleToPlay) return;

	FRotator SpawnRotation = Hit.ImpactNormal.Rotation();

	if (bAttachToHitBone && Hit.Component.IsValid())
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			ParticleToPlay, Hit.Component.Get(), Hit.BoneName,
			Hit.Location, SpawnRotation, EAttachLocation::KeepWorldPosition, true);
	}
	else
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), ParticleToPlay, Hit.Location, SpawnRotation);
	}
}

