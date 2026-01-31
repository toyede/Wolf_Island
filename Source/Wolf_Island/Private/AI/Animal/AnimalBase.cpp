// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Animal/AnimalBase.h"
#include "Perception/AISense_Damage.h"
#include "Components/StatusComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AAnimalBase::AAnimalBase()
{
	PrimaryActorTick.bCanEverTick = true;

	StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));

    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetMesh()->SetCollisionObjectType(ECC_Pawn);
    GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

    GetCharacterMovement()->bUseControllerDesiredRotation = true;
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 180.f, 0.f);  // 천천히 회전
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

    UAISense_Damage::ReportDamageEvent(
        GetWorld(),
        this,
        DamageCauser,
        ActualDamage,
        GetActorLocation(),
        DamageCauser ? DamageCauser->GetActorLocation() : FVector::ZeroVector
    );

    return ActualDamage;
}