// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EnemyAIBase.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/EnemyAIController.h"
#include "Animation/AnimInstance.h"
#include "BehaviorTree/BlackboardComponent.h"


AEnemyAIBase::AEnemyAIBase()
{
    PrimaryActorTick.bCanEverTick = true;

    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -96.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

    // Movement 
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    MoveComp->bUseControllerDesiredRotation = true;
    MoveComp->bOrientRotationToMovement = false;      
    MoveComp->RotationRate = FRotator(0.f, 540.f, 0.f);
    MoveComp->JumpZVelocity = 600.f;
    MoveComp->AirControl = 0.2f;
    MoveComp->MaxWalkSpeed = 600.f;
    MoveComp->BrakingDecelerationWalking = 2048.f;

    // NavAgent
    MoveComp->NavAgentProps.AgentRadius = GetCapsuleComponent()->GetUnscaledCapsuleRadius();       
    MoveComp->NavAgentProps.AgentHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() * 2.f; 

    // AIController 
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = AEnemyAIController::StaticClass();

    // HealthBar Widget
    HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
    HealthBarWidget->SetupAttachment(GetCapsuleComponent());

}

void AEnemyAIBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AEnemyAIBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemyAIBase::ChangeForm(EEnemyForm Form)
{
    if (ChangeFormMontage == nullptr)
    {
        return;
    }

    switch (Form)
    {
    case EEnemyForm::Human:
        if (HumanMesh) GetMesh()->SetSkeletalMesh(HumanMesh);
        break;
    case EEnemyForm::Wolf:
        if (WolfMesh) GetMesh()->SetSkeletalMesh(WolfMesh);
        break;

    default:
        break;
    }

    AEnemyAIController* AICon = Cast<AEnemyAIController>(GetController());

    if (AICon && AICon->GetBlackboardComponent())
    {
        AICon->GetBlackboardComponent()->SetValueAsEnum(AICon->EnemyFormKey, (uint8)Form);
    }
}


