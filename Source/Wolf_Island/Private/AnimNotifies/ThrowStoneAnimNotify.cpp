#include "AnimNotifies/ThrowStoneAnimNotify.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Actors/StoneProjectile.h"

void UThrowStoneAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp || !ProjectileClass) return;

    ACharacter* Owner = Cast<ACharacter>(MeshComp->GetOwner());
    if (!Owner) return;

    // 서버에서만 스폰
    if (!Owner->HasAuthority()) return;

    // 타겟 찾기
    AActor* Target = nullptr;
    if (AAIController* AICon = Cast<AAIController>(Owner->GetController()))
    {
        if (UBlackboardComponent* BB = AICon->GetBlackboardComponent())
        {
            Target = Cast<AActor>(BB->GetValueAsObject("AttackTarget"));
        }
    }
    if (!Target) return;

    // 발사 방향 계산
    FVector HandLoc = MeshComp->GetSocketLocation(TEXT("hand_r"));
    FVector Dir = (Target->GetActorLocation() - HandLoc).GetSafeNormal();

    // 스폰 파라미터 설정
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Owner;
    SpawnParams.Instigator = Owner;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // 스폰 및 발사
    if (UWorld* World = Owner->GetWorld())
    {
        AStoneProjectile* Projectile = World->SpawnActor<AStoneProjectile>(
            ProjectileClass,
            HandLoc,
            Dir.Rotation(),
            SpawnParams
        );

        if (Projectile)
        {
            Projectile->Launch(Dir, 2000.f);
        }
    }
}