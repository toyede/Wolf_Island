// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Animal/AnimalBase.h"
#include "Perception/AISense_Damage.h"
#include "Components/StatusComponent.h"
#include "Components/CapsuleComponent.h"
#include "AI/AIControllers/AnimalController.h"
#include "Net/UnrealNetwork.h"
#include "BrainComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Item/Pickup.h"

AAnimalBase::AAnimalBase()
{
    PrimaryActorTick.bCanEverTick = true;
    StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AAnimalBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAnimalBase::DropItem()
{
	if (!HasAuthority()) return;
	if (!DropItemClass) return;

    FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 50.f);
    FRotator SpawnRotation = FRotator::ZeroRotator;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    APickup* DroppedItem = GetWorld()->SpawnActor<APickup>(DropItemClass, SpawnLocation, SpawnRotation, SpawnParams);

    if (DroppedItem)
    {
        DroppedItem->ItemHandle = DropItemHandle;
        DroppedItem->InitializePickUp(FMath::RandRange(MinDropAmount, MaxDropAmount));
    }
}

void AAnimalBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

float AAnimalBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (ActualDamage <= 0.f)
    {
        return 0.f;
    }

    float OldHP = StatusComponent->CurrentHP;
    StatusComponent->DecreaseHP(ActualDamage);
    float NewHP = StatusComponent->CurrentHP;

    UAISense_Damage::ReportDamageEvent(
        GetWorld(),
        this,
        DamageCauser,
        ActualDamage,
        GetActorLocation(),
        DamageCauser ? DamageCauser->GetActorLocation() : FVector::ZeroVector
    );

    float HalfHP = StatusComponent->MaxHP * 0.5f;

    if (NewHP <= 0)
    {
        if (AAnimalController* AIC = Cast<AAnimalController>(GetController()))
        {
            AIC->SetAnimalState(EAnimalState::Dead);
        }
        Die();
        DropItem();
    }
    else
    {
        if (AAnimalController* AIC = Cast<AAnimalController>(GetController()))
        {
            AIC->SetAnimalState(EAnimalState::Escaping);
        }
    }

    if (NewHP > 0.f && HasAuthority())
    {
        const float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastHitSoundTime >= HitSoundCooldown)
        {
            LastHitSoundTime = CurrentTime;
            MulticastPlayHitSound();
        }
    }

    return ActualDamage;
}

void AAnimalBase::Die()
{
    if (!HasAuthority()) return;

    if (bIsDead) return;

	bIsDead = true;

  MulticastPlayDieSound();

    if (AAnimalController* AIC = Cast<AAnimalController>(GetController()))
    {
        if (AIC->GetBrainComponent())
        {
            AIC->GetBrainComponent()->StopLogic(TEXT("Animal Died"));
		}
    }

	SetLifeSpan(3.0f);
}

void AAnimalBase::OnRep_IsDead()
{
    if (bIsDead)
    {
        ApplyDeadState();
    }
}

void AAnimalBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AAnimalBase, bIsDead);
}

void AAnimalBase::ApplyDeadState()
{
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();
    GetCharacterMovement()->GravityScale = 0.f;

    GetMesh()->GetAnimInstance()->Montage_Stop(0.2f);

    // ĸ��: �ٴڸ� Block, ������ Ignore
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAnimalBase::MulticastPlayHitSound_Implementation()
{
    if (HitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation(), FRotator::ZeroRotator, HitSoundVolumeMultiplier);
    }
}

void AAnimalBase::MulticastPlayDieSound_Implementation()
{
    if (DieSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DieSound, GetActorLocation(), FRotator::ZeroRotator, DieSoundVolumeMultiplier);
    }
}