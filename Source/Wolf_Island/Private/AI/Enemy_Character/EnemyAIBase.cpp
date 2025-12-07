// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Enemy_Character/EnemyAIBase.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/AIControllers/EnemyAIController.h"
#include "Animation/AnimInstance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/StatusComponent.h"
#include "Actors/PatrolRoute.h"
#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"


AEnemyAIBase::AEnemyAIBase()
{
    PrimaryActorTick.bCanEverTick = true;

    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -96.f));

    FaceMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FaceMesh"));
    FaceMesh->SetupAttachment(GetMesh());
    FaceMesh->SetLeaderPoseComponent(GetMesh());

    TorsoMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TorsoMesh"));
    TorsoMesh->SetupAttachment(GetMesh());
    TorsoMesh->SetLeaderPoseComponent(GetMesh());

    LegsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LegsMesh"));
    LegsMesh->SetupAttachment(GetMesh());
    LegsMesh->SetLeaderPoseComponent(GetMesh());

    FeetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FeetMesh"));
    FeetMesh->SetupAttachment(GetMesh());
    FeetMesh->SetLeaderPoseComponent(GetMesh());

    WolfMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WolfMesh"));
    WolfMesh->SetupAttachment(GetCapsuleComponent());
    WolfMesh->SetRelativeLocation(FVector(0.f, 0.f, -96.f));
    WolfMesh->SetVisibility(false);

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

    StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));

    AttackDamage = 10.0f;

}

void AEnemyAIBase::BeginPlay()
{
	Super::BeginPlay();

    HumanParts.Empty();
    HumanParts.Add(FaceMesh);
    HumanParts.Add(TorsoMesh);
    HumanParts.Add(LegsMesh);
    HumanParts.Add(FeetMesh);
	
    if (NativePatrolRoute && WolfPatrolRoute) // 원주민으로 시작해서 기본 루트는 원주민루트로
    {
		AssignedPatrolRoute = NativePatrolRoute;

        CurrentPatrolIndex = GetRandomPointIndex();
    }

    EnemyAIController = Cast<AEnemyAIController>(GetController());

    OnHitResponse.AddDynamic(this, &AEnemyAIBase::HitResponse);
}

void AEnemyAIBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemyAIBase::ChangeForm(EEnemyForm Form)
{
    bIsHuman = (Form == EEnemyForm::Human);

    GetMesh()->SetVisibility(bIsHuman);
    GetMesh()->SetCollisionEnabled(bIsHuman ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);

	GetCapsuleComponent()->SetCapsuleSize(bIsHuman ? 42.f : 70.f, bIsHuman ? 96.f : 96.f); // 캡슐 크기 변경

    // Human 파츠 토글
    TArray<USkeletalMeshComponent*> Parts = { FaceMesh, TorsoMesh, LegsMesh, FeetMesh };
    for (USkeletalMeshComponent* Part : Parts)
    {
        if (Part)
        {
            Part->SetVisibility(bIsHuman);
            Part->SetCollisionEnabled(bIsHuman ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        }
    }

    // Wolf 토글
    if (WolfMesh)
    {
        WolfMesh->SetVisibility(!bIsHuman);
        WolfMesh->SetCollisionEnabled(!bIsHuman ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
    }

    // 패트롤루트 전환
	AssignedPatrolRoute = bIsHuman ? NativePatrolRoute : WolfPatrolRoute;

    // 패트롤 시작점 초기화
	CurrentPatrolIndex = GetRandomPointIndex();

    SpawnParticle();

    // 폼에 따른 패트롤 속도 변화
    PassiveSpeed = bIsHuman ? NativePatrolSpeed : WolfPatrolSpeed;

    // 애니메이션 - GetMesh() 아니고 각각 메시에
    if (bIsHuman && HumanAnimBP)
    {
        GetMesh()->SetAnimInstanceClass(HumanAnimBP);
    }
    else if (!bIsHuman && WolfAnimBP)
    {
        WolfMesh->SetAnimInstanceClass(WolfAnimBP);
    }

    // Blackboard 업데이트
    if (AEnemyAIController* AICon = Cast<AEnemyAIController>(GetController()))
    {
        if (AICon->GetBlackboardComponent())
        {
            AICon->GetBlackboardComponent()->SetValueAsEnum(AICon->EnemyFormKey, (uint8)Form);
        }
    }
}

void AEnemyAIBase::SpawnParticle()
{
    if (FormChangeNiagaraEffect)
    {
        FVector SpawnLocation = GetActorLocation();
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            FormChangeNiagaraEffect,
            SpawnLocation,
            GetActorRotation()
        );
    }
}

int32 AEnemyAIBase::GetNextPoint()
{
    if (AssignedPatrolRoute)
    {
        int32 NumberOfRoutes = AssignedPatrolRoute->SplinePoints->GetNumberOfSplinePoints();

        if (NumberOfRoutes > 0)
        {
            CurrentPatrolIndex = (CurrentPatrolIndex + 1) % NumberOfRoutes;

            return CurrentPatrolIndex;
        }
    }
    return 0;
}

int32 AEnemyAIBase::GetRandomPointIndex()
{
    if (!AssignedPatrolRoute || !AssignedPatrolRoute->SplinePoints)
    {
        return 0;
    }

    int32 NumPoints = AssignedPatrolRoute->SplinePoints->GetNumberOfSplinePoints();
    if (NumPoints <= 0)
    {
        return 0;
    }

    return FMath::RandRange(-1, AssignedPatrolRoute->SplinePoints->GetNumberOfSplinePoints() - 1); // 원주민마다 랜덤 스타트

}

void AEnemyAIBase::StopAllMontages()
{
    // Human Mesh
    if (UAnimInstance* HumanAnim = GetMesh()->GetAnimInstance())
    {
        HumanAnim->Montage_Stop(0.2f);
    }

    // Wolf Mesh
    if (WolfMesh)
    {
        if (UAnimInstance* WolfAnim = WolfMesh->GetAnimInstance())
        {
            WolfAnim->Montage_Stop(0.2f);
        }
    }
}

void AEnemyAIBase::Growling()
{
    if (GrowlSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this,
            GrowlSound,
            GetActorLocation(),
            FRotator::ZeroRotator,
            1.0f,   // Volume
            1.0f,   // Pitch
            0.0f,   // Start Time
            AISoundAttenuation);
    }
}

// Interface Functions

void AEnemyAIBase::SetMovementSpeed_Implementation(EEnemyState State)
{
    switch (State)
    {
        case EEnemyState::Passive:
            GetCharacterMovement()->MaxWalkSpeed = PassiveSpeed;
            break;
        case EEnemyState::Attacking:
            GetCharacterMovement()->MaxWalkSpeed = AttackingSpeed;
            break;
        case EEnemyState::Dead:
            GetCharacterMovement()->MaxWalkSpeed = DeadSpeed;
            break;
        default:
            break;
    }
}

void AEnemyAIBase::ThrowObject_Implementation()
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance || !ThrowMontage)
    {
        return;
    }

    AnimInstance->Montage_Play(ThrowMontage);

    if (ThrowSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ThrowSound, GetActorLocation());
    }
}

UAnimMontage* AEnemyAIBase::GetThrowMontage_Implementation()
{
    return ThrowMontage;
}

void AEnemyAIBase::HitResponse()
{
    EnemyAIController->SetStateAsFrozen();

    StopAllMontages();
    GetCharacterMovement()->StopMovementImmediately();

    UAnimInstance* AnimInstance = nullptr;
    UAnimMontage* MontageToPlay = nullptr;

    if (bIsHuman)
    {
        AnimInstance = GetMesh()->GetAnimInstance();
        MontageToPlay = FrozenMontage_Native;
    }
    else
    {
        AnimInstance = WolfMesh->GetAnimInstance();
        MontageToPlay = FrozenMontage_Wolf;
    }

    if (AnimInstance && MontageToPlay)
    {
        AnimInstance->Montage_Play(MontageToPlay);

        // 몽타주 끝나면 Attacking으로 전환
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &AEnemyAIBase::OnFrozenMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
    }

    if (FrozenHitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FrozenHitSound, GetActorLocation());
    }
}

void AEnemyAIBase::OnFrozenMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (EnemyAIController && EnemyAIController->EnemyState != EEnemyState::Dead)
    {
        if (EnemyAIController->AttackTarget)
        {
            EnemyAIController->SetStateAsAttacking(EnemyAIController->AttackTarget, true);
        }
        else
        {
            EnemyAIController->SetStateAsPassive();
        }
    }
}

float AEnemyAIBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    StatusComponent->DecreaseHP(ActualDamage);

    // 죽으면 HitResponse 안 하고 바로 Dead
    if (StatusComponent->CurrentHP <= 0)
    {
        EnemyAIController->SetStateAsDead();

        //DEAD 상태면 모든 몽타주 중단 + EndDelegate 제거
        StopAllMontages();
        FOnMontageEnded EmptyDelegate;
        if (UAnimInstance* HumanAnim = GetMesh()->GetAnimInstance())
            HumanAnim->Montage_SetEndDelegate(EmptyDelegate, nullptr);

        if (WolfMesh)
        {
            if (UAnimInstance* WolfAnim = WolfMesh->GetAnimInstance())
                WolfAnim->Montage_SetEndDelegate(EmptyDelegate, nullptr);
        }

        return ActualDamage;
    }

    else
    {
        OnHitResponse.Broadcast();
    }

    return ActualDamage;
}




