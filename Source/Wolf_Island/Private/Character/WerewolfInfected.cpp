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

AWerewolfInfected::AWerewolfInfected()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    AttackCollisionComp = CreateDefaultSubobject<UAttackCollisionComponent>(
        TEXT("AttackCollision"));

    Tags.Add(FName("Werewolf")); // 늑대들이 때리지 않도록
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


// === 데미지 & 기절 ===

float AWerewolfInfected::TakeDamage(float DamageAmount,
    FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsIncapacitated) return 0.0f;  // 무적
    if (!HasAuthority()) return 0.0f;

    float ActualDamage = Super::TakeDamage(
        DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    // 10% 이하면 기절
    if (CurrentHealth <= MaxHealth * IncapacitateThreshold)
    {
        HandleIncapacitated();
    }
    else
    {
        // 살아있다면 피격 리액션 실행
        HitResponse();
    }

    return ActualDamage;
}

void AWerewolfInfected::NormalAttack_Implementation()
{
    if (HasAuthority())
    {
        // 서버에서 멀티캐스트 호출
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
    // MoonlightInfectionSystem을 찾아 기절 처리 요청
    AMoonlightInfectionSystem* System = Cast<AMoonlightInfectionSystem>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AMoonlightInfectionSystem::StaticClass()));

    if (System)
    {
        // 이 늑대인간을 소유한 PlayerController를 찾아 전달
        // (세션 데이터에 등록된 PC를 찾기 위한 용도)
        System->NotifyWerewolfDown(this);
    }

    OnRep_Incapacitated();
}

void AWerewolfInfected::OnRep_Incapacitated()
{
    if (bIsIncapacitated)
    {
        // 충돌 무적 처리
        GetCapsuleComponent()->SetCollisionResponseToChannel(
            ECC_GameTraceChannel1, ECR_Ignore);  // 공격 채널

        // 입력 비활성화
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
    // UI 업데이트 등 클라이언트 측 반응
}

// === 공격 ===

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
    GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "z");

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance || !AttackMontage)
    {
        if (HasAuthority())
        {
            OnAttackEnd.Broadcast();
        }
        return;
    }

    // 공격 중 상태 설정
    bIsAttacking = true;

    // 몽타주 재생
    AnimInstance->Montage_Play(AttackMontage);

    // 공격 사운드 (EnemyAIBase 스타일 추가)
    /*if (AttackSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
    }*/

    // 서버인 경우 몽타주 종료 델리게이트 바인딩
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
        // AttackCollisionComp 활성화는 AnimNotify에서 처리
    }
}

void AWerewolfInfected::OnAttackHit(const FHitResult& HitResult)
{
    if (!HasAuthority()) return;

    AActor* HitActor = HitResult.GetActor();
    if (!HitActor) return;

    GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
		FString::Printf(TEXT("Hit: %s"), *HitActor->GetName()));

    TSubclassOf<UDamageType> DamageTypeClass = UDamageType::StaticClass();
    if (InfectedAttackDamageType)
    {
        DamageTypeClass = InfectedAttackDamageType;
    }

    UGameplayStatics::ApplyDamage(HitActor, AttackDamage, GetController(), this, DamageTypeClass);
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

        // Behavior Tree에서 공격이나 추적을 멈추게 할 Blackboard Key 설정 (예: bIsHit)
        if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
        {
            BB->SetValueAsBool(FName("bIsHit"), true);
        }
    }

    Multicast_HitResponse();
}

void AWerewolfInfected::Multicast_HitResponse_Implementation()
{
    // 진행 중인 애니메이션(공격 등) 강제 종료
    if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    {
        AnimInstance->Montage_Stop(0.1f);
    }

    GetCharacterMovement()->StopMovementImmediately();
    bIsAttacking = false; // 공격 상태 강제 해제 (공격 콜리전 무효화)

    // 피격 몽타주 재생
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