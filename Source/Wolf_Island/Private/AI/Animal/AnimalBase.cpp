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

AAnimalBase::AAnimalBase()
{
    PrimaryActorTick.bCanEverTick = true;
    StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Overlap);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetMesh()->SetCollisionObjectType(ECC_Pawn);
    GetMesh()->SetGenerateOverlapEvents(true);
}

void AAnimalBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAnimalBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

float AAnimalBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

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
    }
    // ¹ÝÇÇ ÀÌ»ó ¡æ ¹ÝÇÇ ÀÌÇÏ·Î ¶³¾îÁö´Â ¼ø°£¸¸ µµ¸Á
    else if (OldHP > HalfHP && NewHP <= HalfHP)
    {
        if (AAnimalController* AIC = Cast<AAnimalController>(GetController()))
        {
            AIC->SetAnimalState(EAnimalState::Escaping);
        }
    }

    return ActualDamage;
}

void AAnimalBase::Die()
{
    if (!HasAuthority()) return;

    if (bIsDead) return;

	bIsDead = true;

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

    // Ä¸½¶: ¹Ù´Ú¸¸ Block, ³ª¸ÓÁö Ignore
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (DieSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DieSound, GetActorLocation());
    }
}