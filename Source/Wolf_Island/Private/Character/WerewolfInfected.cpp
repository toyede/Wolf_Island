#include "Character/WerewolfInfected.h"
#include "Net/UnrealNetwork.h"
#include "Components/AttackCollisionComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/MainPlayer.h"
#include "Components/StatusComponent.h"
#include "Kismet/GameplayStatics.h"

AWerewolfInfected::AWerewolfInfected()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;

    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(
        TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetMesh(), TEXT("headSocket"));
    FirstPersonCamera->bUsePawnControlRotation = true;
    FirstPersonCamera->SetRelativeTransform(
        FTransform(
            FRotator(0, 90, -90),
            FVector(0, 10, 0)
        ));

    AttackCollisionComp = CreateDefaultSubobject<UAttackCollisionComponent>(
        TEXT("AttackCollision"));

    Tags.Add(FName("Werewolf")); // 늑대들이 때리지 않도록
}

void AWerewolfInfected::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;

    // 서버에서만 AI 로직 실행
    if (HasAuthority())
    {
        GetWorldTimerManager().SetTimer(
            AITickHandle, this,
            &AWerewolfInfected::AITick,
            AITickInterval, true
        );
    }
}

void AWerewolfInfected::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!HasAuthority()) return;
    if (bIsIncapacitated || bIsAttacking) return;

    if (bShouldMove && CurrentTarget.IsValid())
    {
        FVector Direction = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        FRotator LookRotation = Direction.Rotation();

        // 컨트롤러 회전으로 변경
        GetController()->SetControlRotation(FRotator(0, LookRotation.Yaw, 0));

        AddMovementInput(Direction, 1.0f);
    }
}

void AWerewolfInfected::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AWerewolfInfected, CurrentHealth);
    DOREPLIFETIME(AWerewolfInfected, bIsIncapacitated);
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

    return ActualDamage;
}

void AWerewolfInfected::HandleIncapacitated()
{
    bIsIncapacitated = true;
    bShouldMove = false;

    // AI 타이머 정지
    GetWorldTimerManager().ClearTimer(AITickHandle);
    OnRep_Incapacitated();  // 서버에서도 즉시 적용
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

        // TODO: 기절 몽타주 재생
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
    // Enhanced Input 사용 시 여기서 공격 액션 바인딩
    // 기존 MainPlayer의 입력 시스템과 맞춰서 구성
}

void AWerewolfInfected::OnAttackInput()
{
    if (bIsIncapacitated) return;
    Server_RequestAttack();
}

void AWerewolfInfected::Multicast_PlayAttack_Implementation()
{
    if (AttackMontage)
    {
        PlayAnimMontage(AttackMontage);
    }
}

void AWerewolfInfected::AITick()
{
    if (bIsIncapacitated) return;

    if (!CurrentTarget.IsValid() || CurrentTarget->IsActorBeingDestroyed())
    {
        FindClosestPlayer();
    }

    if (CurrentTarget.IsValid())
    {
        float Distance = FVector::Dist(
            GetActorLocation(), CurrentTarget->GetActorLocation());

        if (Distance <= AttackRange)
        {
            bShouldMove = false;
            TryAttack();
        }
        else
        {
            bShouldMove = true;
        }
    }
    else
    {
        bShouldMove = false;
    }
}

void AWerewolfInfected::FindClosestPlayer()
{
    float ClosestDist = DetectionRange;
    ACharacter* ClosestPlayer = nullptr;

    TArray<AActor*> FoundPlayers;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMainPlayer::StaticClass(), FoundPlayers);

    for (AActor* Actor : FoundPlayers)
    {
        AMainPlayer* MP = Cast<AMainPlayer>(Actor);
        if (!MP) continue;

        // 숨겨진 캐릭터 제외 (변신한 플레이어의 원래 캐릭터)
        if (MP->IsHidden()) continue;

        // 기절/사망 제외
        if (MP->StatusComponent && MP->StatusComponent->bIsIncapacitated) continue;

        float Dist = FVector::Dist(GetActorLocation(), MP->GetActorLocation());
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestPlayer = MP;
        }
    }

    CurrentTarget = ClosestPlayer;
}

void AWerewolfInfected::TryAttack()
{
    if (bIsAttacking || !AttackMontage) return;

    bIsAttacking = true;

    float Duration = PlayAnimMontage(AttackMontage);
    Multicast_PlayAttack();

    if (Duration <= 0.0f)
    {
        bIsAttacking = false;
        return;
    }

    GetWorldTimerManager().ClearTimer(AttackResetHandle);
    GetWorldTimerManager().SetTimer(
        AttackResetHandle,
        [this]() { bIsAttacking = false; },
        Duration, false
    );
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