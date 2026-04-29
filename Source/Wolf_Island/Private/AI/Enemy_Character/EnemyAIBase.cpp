// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Enemy_Character/EnemyAIBase.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/AIControllers/EnemyAIController.h"
#include "Animation/AnimInstance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/StatusComponent.h"
#include "Components/AttackCollisionComponent.h"
#include "Actors/PatrolRoute.h"
#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "BrainComponent.h"
#include "Actors/Interfaces/SkyInterface.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Damage.h"
#include "Item/Pickup.h"

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

    // Components
    StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));
    AttackCollisionComponent = CreateDefaultSubobject<UAttackCollisionComponent>(TEXT("AttackCollisionComponent"));

    AttackDamage = 10.0f;

    bReplicates = true;
    SetReplicateMovement(true);

}

void AEnemyAIBase::BeginPlay()
{
	Super::BeginPlay();

    if (StatusComponent)
    {
        BaseHumanArmor = StatusComponent->Armor;
    }

    if (HasAuthority())
    {
        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("SkyManager"), Found);

        for (AActor* Actor : Found)
        {
            if (IsValid(Actor) && Actor->Implements<USkyInterface>())
            {
                CachedSkyManager = Actor;
                break;
            }
        }
        EnemyAIController = Cast<AEnemyAIController>(GetController());

        if (EnemyAIController)
        {
            EnemyAIController->OnEnemyStateChanged.AddDynamic(this, &AEnemyAIBase::OnStateChanged);
        }
    }

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

    if (AttackCollisionComponent)
    {
        AttackCollisionComponent->OnHitActor.AddUObject(this, &AEnemyAIBase::OnAttackHit);
        AttackCollisionComponent->AddIgnoredActor(this);
    }
}

void AEnemyAIBase::OnRep_Controller()
{
    Super::OnRep_Controller();
    
    // 클라이언트에서 Controller가 도착한 시점에 Movement 상태를 리셋
    if (GetController())
    {
        if (UCharacterMovementComponent* CMC = GetCharacterMovement())
        {
            // Simulated Proxy의 Movement 상태를 강제로 갱신
            CMC->SetUpdatedComponent(GetRootComponent());
            
            // 현재 서버 위치로 스냅
            if (GetRootComponent())
            {
                GetRootComponent()->UpdateComponentToWorld();
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("[Client] %s Controller replicated: %s"),
            *GetName(), *GetController()->GetName());
    }
}

void AEnemyAIBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemyAIBase::NotifySkyRemoveSelf()
{
    if (!HasAuthority()) return;

        if (CachedSkyManager && CachedSkyManager->Implements<USkyInterface>())
        {
            ISkyInterface::Execute_RemoveEnemy(CachedSkyManager, this);
        }
}

void AEnemyAIBase::OnRep_EnemyForm()
{
    ApplyFormVisuals();
}

void AEnemyAIBase::ServerChangeForm_Implementation(EEnemyForm Form)
{
	EnemyForm = Form;

    ApplyFormVisuals();

    if (AEnemyAIController* AIC = Cast<AEnemyAIController>(GetController()))
    {
        if (AIC->GetBlackboardComponent())
        {
            AIC->GetBlackboardComponent()->SetValueAsEnum(AIC->EnemyFormKey, (uint8)Form);
        }
	}
}

void AEnemyAIBase::ApplyFormVisuals()
{
    bIsHuman = (EnemyForm == EEnemyForm::Human);
    ApplyFormDefense();

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

    // 애니메이션 - GetMesh() 아니고 각각 메시에
    if (bIsHuman && HumanAnimBP)
    {
        GetMesh()->SetAnimInstanceClass(HumanAnimBP);
    }
    else if (!bIsHuman && WolfAnimBP)
    {
        WolfMesh->SetAnimInstanceClass(WolfAnimBP);
    }
}

void AEnemyAIBase::ApplyFormDefense()
{
    if (!StatusComponent)
    {
        return;
    }

    StatusComponent->Armor = bIsHuman ? BaseHumanArmor : (BaseHumanArmor + WolfArmorBonus);
}

void AEnemyAIBase::ApplySpeedByState(EEnemyState State)
{
    float SpeedToApply = 0.f;

    switch (State)
    {
    case EEnemyState::Passive:
        // PassiveSpeed 변수 대신 직접 계산 (클라이언트도 동일한 값 보유)
        SpeedToApply = bIsHuman ? NativePatrolSpeed : WolfPatrolSpeed;
        break;
    case EEnemyState::Combat:
        SpeedToApply = AttackingSpeed;
        break;
    case EEnemyState::Dead:
		SpeedToApply = DeadSpeed;
		break;
    case EEnemyState::Frozen:
        break;
    case EEnemyState::Investigating:
        SpeedToApply = bIsHuman ? NativePatrolSpeed : WolfPatrolSpeed;
        break;
    default:
        return;
    }

    GetCharacterMovement()->MaxWalkSpeed = SpeedToApply;
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

void AEnemyAIBase::OnThrowMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!bInterrupted)
    {
        OnThrowEnd.Broadcast();
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
    ApplySpeedByState(State);
}

void AEnemyAIBase::ThrowObject_Implementation()
{
    if (HasAuthority())
    {
        Multicast_PlayThrowMontage();
    }
}
void AEnemyAIBase::Die_Implementation()
{
    if (!HasAuthority())
        return;

    if (bIsDead)
        return;

    bIsDead = true;

    // relevancy 밖에 있던 클라에게도 빠르게 상태를 밀어주기
    FlushNetDormancy();
    ForceNetUpdate();

    // 서버에서도 같은 비주얼/콜리전 상태 적용(서버는 OnRep가 자동으로 안 돈다고 보는 게 안전)
    ApplyDeadState();

    // 서버 전용 로직은 여기서만
    NotifySkyRemoveSelf();

    if (EnemyAIController && EnemyAIController->GetBrainComponent())
    {
        EnemyAIController->GetBrainComponent()->StopLogic(TEXT("Enemy Dead"));
    }

    SetLifeSpan(2.5f);
}


void AEnemyAIBase::NormalAttack_Implementation()
{
    if (HasAuthority())
    {
		Multicast_PlayNormalAttackMontage();
    }
}

void AEnemyAIBase::Howling_Implementation()
{
    if (HasAuthority())
    {
        Multicast_PlayHowlingMontage();

        if (AEnemyAIController* AICon = Cast<AEnemyAIController>(GetController()))
        {
            UAISense_Hearing::ReportNoiseEvent(
                GetWorld(),
                GetActorLocation(),
                1.0f,
                this,
                0.0f,
                TEXT("Howling")
			);
        }
    }
}

void AEnemyAIBase::Heal()
{
	StatusComponent->IncreaseHP(HealAmount);
}

void AEnemyAIBase::HitResponse()
{
    if (!HasAuthority()) return;

    if (!EnemyAIController) return;

    EnemyAIController->SetEnemyState(EEnemyState::Frozen);

    Multicast_HitResponse();
}

void AEnemyAIBase::Multicast_HitResponse_Implementation()
{
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

        if (HasAuthority())
        {
            FOnMontageEnded EndDelegate;
            EndDelegate.BindUObject(this, &AEnemyAIBase::OnFrozenMontageEnded);
            AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);
        }
    }

    if (FrozenHitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FrozenHitSound, GetActorLocation());
    }
}
void AEnemyAIBase::OnFrozenMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!EnemyAIController || EnemyAIController->EnemyState == EEnemyState::Dead)
    {
        return;
    }

    if (EnemyAIController->AttackTarget)
    {
        EnemyAIController->SetEnemyState(EEnemyState::Combat);
    }
    else
    {
        EnemyAIController->SetEnemyState(EEnemyState::Passive);
    }
}

void AEnemyAIBase::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!bInterrupted)
    {
        OnAttackEnd.Broadcast();
    }
   
}

void AEnemyAIBase::OnHowlingMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!bInterrupted)
    {
        OnHowlingEnd.Broadcast();
    }
}

void AEnemyAIBase::OnAttackHit(const FHitResult& HitResult)
{
    AActor* HitActor = HitResult.GetActor();
    if (!HitActor || !HasAuthority())
    {
        return;
    }

    TSubclassOf<UDamageType> DamageTypeClass = UDamageType::StaticClass();

    if (!bIsHuman && WolfAttackDamageType)
    {
        DamageTypeClass = WolfAttackDamageType;
    }
    
    UGameplayStatics::ApplyDamage(HitActor, AttackDamage, GetController(), this, DamageTypeClass);
}

float AEnemyAIBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (DamageCauser && DamageCauser->IsA<AEnemyAIBase>())
    {
        return 0.f;
    }
    float OldHP = StatusComponent->CurrentHP;
    StatusComponent->DecreaseHP(ActualDamage);
    float NewHP = StatusComponent->CurrentHP;
    float HalfHP = StatusComponent->MaxHP * 0.5f;

    UAISense_Damage::ReportDamageEvent(
        GetWorld(),
        this,
        DamageCauser,
        ActualDamage,
        GetActorLocation(),
        DamageCauser ? DamageCauser->GetActorLocation() : FVector::ZeroVector
	);

    if (OldHP > HalfHP && NewHP <= HalfHP)
    {
        if (AEnemyAIController* AIC = Cast<AEnemyAIController>(GetController()))
        {
			AIC->GetBlackboardComponent()->SetValueAsBool(FName("bIsHalfHP"), true);
        }
    }

    if (StatusComponent->CurrentHP <= 0)
    {
        if (EnemyAIController)
        {
            EnemyAIController->SetEnemyState(EEnemyState::Dead);
        }

        FOnMontageEnded EmptyDelegate;
        if (UAnimInstance* HumanAnim = GetMesh()->GetAnimInstance())
            HumanAnim->Montage_SetEndDelegate(EmptyDelegate, nullptr);

        if (WolfMesh)
        {
            if (UAnimInstance* WolfAnim = WolfMesh->GetAnimInstance())
                WolfAnim->Montage_SetEndDelegate(EmptyDelegate, nullptr);
        }

		DropItem();
    }
    else
    {
        OnHitResponse.Broadcast();
    }

    return ActualDamage;
}

void AEnemyAIBase::Multicast_PlayThrowMontage_Implementation()
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance || !ThrowMontage)
    {
        if (HasAuthority())
        {
            OnThrowEnd.Broadcast();
        }
        return;
    }

    AnimInstance->Montage_Play(ThrowMontage);

    if (ThrowSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ThrowSound, GetActorLocation());
    }

    if (HasAuthority())
    {
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &AEnemyAIBase::OnThrowMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, ThrowMontage);
    }
}

void AEnemyAIBase::Multicast_PlayNormalAttackMontage_Implementation()
{
    UAnimInstance* AnimInstance = WolfMesh->GetAnimInstance();

    if (!AnimInstance || !AttackMontage)
    {
        if (HasAuthority())
        {
            OnAttackEnd.Broadcast();
        }
        return;
    }

    AnimInstance->Montage_Play(AttackMontage);

    if (AttackSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
    }

    if (HasAuthority())
    {
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &AEnemyAIBase::OnAttackMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
    }
}

void AEnemyAIBase::Multicast_PlayHowlingMontage_Implementation()
{
    UAnimInstance* AnimInstance = WolfMesh->GetAnimInstance();

    if (!AnimInstance || !HowlingMontage)
    {
        if (HasAuthority())
        {
            OnHowlingEnd.Broadcast();
        }
        return;
    }

    AnimInstance->Montage_Play(HowlingMontage);

    if (HowlingSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, HowlingSound, GetActorLocation());
    }

    if (HasAuthority())
    {
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &AEnemyAIBase::OnHowlingMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, HowlingMontage);
    }
}

void AEnemyAIBase::OnRep_IsDead()
{
    if (bIsDead)
    {
        ApplyDeadState();
    }
}

void AEnemyAIBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AEnemyAIBase, bIsDead);
	DOREPLIFETIME(AEnemyAIBase, EnemyForm);
}

void AEnemyAIBase::ApplyDeadState()
{
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();
    GetCharacterMovement()->GravityScale = 0.f;

    StopAllMontages();

    // 캡슐: 바닥만 Block, 나머지 Ignore
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (WolfMesh)
    {
        WolfMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (DieSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DieSound, GetActorLocation());
    }
}

void AEnemyAIBase::OnStateChanged(EEnemyState NewState)
{
    if (NewState == EEnemyState::Passive)
    {
        // 힐 타이머 시작
        GetWorld()->GetTimerManager().SetTimer(
            HealTimer,
            this,
            &AEnemyAIBase::Heal,
            HealInterval,  // 예: 2초마다
            true           // 반복
        );
    }
    else
    {
        // 다른 상태면 타이머 클리어
        GetWorld()->GetTimerManager().ClearTimer(HealTimer);
    }
}

void AEnemyAIBase::DropItem()
{
    if (!HasAuthority()) return;
    if (!DropItemClass) return;
	if (EnemyForm != EEnemyForm::Wolf) return; // 아이템 드랍은 늑대 형태에서만

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

