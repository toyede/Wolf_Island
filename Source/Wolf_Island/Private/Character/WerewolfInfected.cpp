#include "Character/WerewolfInfected.h"
#include "Net/UnrealNetwork.h"
#include "Components/AttackCollisionComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/MainPlayer.h"
#include "Components/StatusComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AIController.h"
#include "Engine/DamageEvents.h"
#include "Moon/MoonlightInfectionSystem.h"
#include "AI/AIControllers/InfectedAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystem.h"

AWerewolfInfected::AWerewolfInfected()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    AttackCollisionComp = CreateDefaultSubobject<UAttackCollisionComponent>(
        TEXT("AttackCollision"));

    Tags.Add(FName("Werewolf")); // ������� ������ �ʵ���
}

void AWerewolfInfected::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;

    if (AttackCollisionComp)
    {
        AttackCollisionComp->OnHitActor.AddUObject(this, &AWerewolfInfected::OnAttackHit);
        AttackCollisionComp->AddIgnoredActor(this);
    }
}

void AWerewolfInfected::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWerewolfInfected::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AWerewolfInfected, CurrentHealth);
	DOREPLIFETIME(AWerewolfInfected, bIsIncapacitated);
}

void AWerewolfInfected::Die_Implementation()
{
}


// === ������ & ���� ===

float AWerewolfInfected::TakeDamage(float DamageAmount,
    FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsIncapacitated) return 0.0f;  // ����
    if (!HasAuthority()) return 0.0f;

    float ActualDamage = Super::TakeDamage(
        DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    // 피격 사운드 + 이펙트 — Unreliable Multicast (cosmetic, HitResponse 확률 없이 무조건)
    const FVector HitLoc = DamageCauser ? DamageCauser->GetActorLocation() : GetActorLocation();
    const FVector HitNorm = (GetActorLocation() - HitLoc).GetSafeNormal();
    Multicast_PlayHitEffect(GetActorLocation(), HitNorm);

    // 10% 이하면 기절
    if (CurrentHealth <= MaxHealth * IncapacitateThreshold)
    {
        HandleIncapacitated();
    }
    else
    {
        // ����ִٸ� �ǰ� ���׼� ����
        HitResponse();
    }

    return ActualDamage;
}

void AWerewolfInfected::NormalAttack_Implementation()
{
    if (HasAuthority())
    {
        // �������� ��Ƽĳ��Ʈ ȣ��
        Multicast_PlayAttack();
    }
}

void AWerewolfInfected::HandleIncapacitated()
{
    bIsIncapacitated = true;

    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        if (UBlackboardComponent* BB = AIC->GetBlackboardComponent()) {
            BB->SetValueAsBool(FName("bIsIncapacitated"), true);
        }
		AIC->StopMovement();
	}
    // MoonlightInfectionSystem�� ã�� ���� ó�� ��û
    AMoonlightInfectionSystem* System = Cast<AMoonlightInfectionSystem>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AMoonlightInfectionSystem::StaticClass()));

    if (System)
    {
        // �� �����ΰ��� ������ PlayerController�� ã�� ����
        // (���� �����Ϳ� ��ϵ� PC�� ã�� ���� �뵵)
        System->NotifyWerewolfDown(this);
    }

    OnRep_Incapacitated();
}

void AWerewolfInfected::OnRep_Incapacitated()
{
    if (bIsIncapacitated)
    {
        // �浹 ���� ó��
        GetCapsuleComponent()->SetCollisionResponseToChannel(
            ECC_GameTraceChannel1, ECR_Ignore);  // ���� ä��

        // �Է� ��Ȱ��ȭ
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            DisableInput(PC);
        }

        if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
        {
            AnimInstance->Montage_Stop(0.1f);
        }
    }
}

void AWerewolfInfected::OnRep_CurrentHealth()
{
    // UI ������Ʈ �� Ŭ���̾�Ʈ �� ����
}

// === ���� ===

void AWerewolfInfected::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(SpectateIMC, 0);
        }
    }

    UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EIC) return;

    EIC->BindAction(SwitchSpectateAction, ETriggerEvent::Started, this, &AWerewolfInfected::SwitchSpectateTarget);
}

void AWerewolfInfected::OnAttackInput()
{
    if (bIsIncapacitated) return;
    Server_RequestAttack();
}

void AWerewolfInfected::Multicast_PlayAttack_Implementation()
{
    //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "z");

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance || !AttackMontage)
    {
        if (HasAuthority())
        {
            OnAttackEnd.Broadcast();
        }
        return;
    }

    // ���� �� ���� ����
    bIsAttacking = true;

    // ��Ÿ�� ���
    AnimInstance->Montage_Play(AttackMontage);

    // ���� ���� (EnemyAIBase ��Ÿ�� �߰�)
    /*if (AttackSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
    }*/

    // ������ ��� ��Ÿ�� ���� ��������Ʈ ���ε�
    if (HasAuthority())
    {
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &AWerewolfInfected::OnAttackMontageEnded);
        AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);
    }
}

void AWerewolfInfected::SwitchSpectateTarget()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    TArray<AActor*> FoundPlayers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMainPlayer::StaticClass(), FoundPlayers);

    AActor* CurrentView = PC->GetViewTarget();

    TArray<AMainPlayer*> ValidTargets;
    for (AActor* Actor : FoundPlayers)
    {
        AMainPlayer* MP = Cast<AMainPlayer>(Actor);
        if (!MP || MP->IsHidden()) continue;
        ValidTargets.Add(MP);
    }

    if (ValidTargets.Num() == 0) return;

    int32 CurrentIndex = ValidTargets.IndexOfByKey(CurrentView);
    int32 NextIndex = (CurrentIndex + 1) % ValidTargets.Num();

    PC->SetViewTargetWithBlend(ValidTargets[NextIndex], 0.3f);
}

USkeletalMeshComponent* AWerewolfInfected::GetAttackMesh() const
{
    return GetMesh();
}

UAttackCollisionComponent* AWerewolfInfected::GetAttackCollisionComponent() const
{
    return AttackCollisionComp;
}

void AWerewolfInfected::Server_RequestAttack_Implementation()
{
    if (bIsIncapacitated) return;

    if (AttackMontage)
    {
        PlayAnimMontage(AttackMontage);
        // AttackCollisionComp Ȱ��ȭ�� AnimNotify���� ó��
    }
}

void AWerewolfInfected::OnAttackHit(const FHitResult& HitResult)
{
    if (!HasAuthority()) return;

    AActor* HitActor = HitResult.GetActor();
    if (!HitActor) return;

    //GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
	//	FString::Printf(TEXT("Hit: %s"), *HitActor->GetName()));

    TSubclassOf<UDamageType> DamageTypeClass = UDamageType::StaticClass();
    if (InfectedAttackDamageType)
    {
        DamageTypeClass = InfectedAttackDamageType;
    }

    //KSH-HitParticleComponent가 피격 지점/노멀을 쓸 수 있도록 PointDamage로 전달
    UGameplayStatics::ApplyPointDamage(
        HitActor,
        AttackDamage,
        -HitResult.ImpactNormal,
        HitResult,
        GetController(),
        this,
        DamageTypeClass);
}

void AWerewolfInfected::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    bIsAttacking = false;

    if (!bInterrupted)
    {
        OnAttackEnd.Broadcast();
    }
}

void AWerewolfInfected::HitResponse()
{
    if (!HasAuthority()) return;

    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        AIC->StopMovement();

        // Behavior Tree���� �����̳� ������ ���߰� �� Blackboard Key ���� (��: bIsHit)
        if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
        {
            BB->SetValueAsBool(FName("bIsHit"), true);
        }
    }

    Multicast_HitResponse();
}

void AWerewolfInfected::Multicast_HitResponse_Implementation()
{
    // ���� ���� �ִϸ��̼�(���� ��) ���� ����
    if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    {
        AnimInstance->Montage_Stop(0.1f);
    }

    GetCharacterMovement()->StopMovementImmediately();
    bIsAttacking = false; // ���� ���� ���� ���� (���� �ݸ��� ��ȿȭ)

    // �ǰ� ��Ÿ�� ���
    if (HitMontage)
    {
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
        if (AnimInstance)
        {
            AnimInstance->Montage_Play(HitMontage);

            if (HasAuthority())
            {
                FOnMontageEnded EndDelegate;
                EndDelegate.BindUObject(this, &AWerewolfInfected::OnHitMontageEnded);
                AnimInstance->Montage_SetEndDelegate(EndDelegate, HitMontage);
            }
        }
    }
    else
    {
        if (HasAuthority())
        {
            OnHitMontageEnded(nullptr, false);
        }
    }
}

void AWerewolfInfected::OnHitMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (bIsIncapacitated) return;

    if (AAIController* AIC = Cast<AAIController>(GetController()))
    {
        if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
        {
            BB->SetValueAsBool(FName("bIsHit"), false);
        }
    }
}

void AWerewolfInfected::Multicast_PlayHitEffect_Implementation(FVector HitLocation, FVector HitNormal)
{
    // 피격 사운드
    if (HitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, HitSound, HitLocation);
    }

    // Niagara 우선, 없으면 Cascade 폴백
    if (HitEffect)
    {
        if (HitEffectSocketName != NAME_None && GetMesh())
        {
            UNiagaraFunctionLibrary::SpawnSystemAttached(
                HitEffect, GetMesh(), HitEffectSocketName,
                FVector::ZeroVector, HitNormal.Rotation(),
                EAttachLocation::KeepRelativeOffset, true);
        }
        else
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(), HitEffect, HitLocation, HitNormal.Rotation());
        }
    }
    else if (HitEffectCascade)
    {
        if (HitEffectSocketName != NAME_None && GetMesh())
        {
            UGameplayStatics::SpawnEmitterAttached(
                HitEffectCascade, GetMesh(), HitEffectSocketName,
                FVector::ZeroVector, HitNormal.Rotation(),
                EAttachLocation::KeepRelativeOffset);
        }
        else
        {
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(), HitEffectCascade, HitLocation, HitNormal.Rotation());
        }
    }
}
