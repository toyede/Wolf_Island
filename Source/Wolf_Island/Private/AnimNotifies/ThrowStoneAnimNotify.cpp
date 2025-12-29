// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifies/ThrowStoneAnimNotify.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

void UThrowStoneAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    ACharacter* Owner = Cast<ACharacter>(MeshComp->GetOwner());
    if (!Owner || !ProjectileClass) return;

    if (!Owner->HasAuthority()) return;

    AActor* Target = nullptr;
    if (AAIController* AICon = Cast<AAIController>(Owner->GetController()))
    {
        UBlackboardComponent* BB = AICon->GetBlackboardComponent();
        if (BB)
        {
            Target = Cast<AActor>(BB->GetValueAsObject("AttackTarget"));
            GEngine->AddOnScreenDebugMessage(1, 1, FColor::Black, *Target->GetName());
        }
    }
    if (!Target) return;

    FVector HandLoc = MeshComp->GetSocketLocation(TEXT("hand_r"));
    FVector Dir = (Target->GetActorLocation() - HandLoc).GetSafeNormal();
    FRotator Rot = Dir.Rotation();

    UWorld* World = Owner->GetWorld();
    if (World)
    {
        AStoneProjectile* Projectile = World->SpawnActor<AStoneProjectile>(ProjectileClass, HandLoc, Rot);
        if (Projectile)
        {
            Projectile->Mesh->IgnoreActorWhenMoving(Owner, true);
            Projectile->SetOwner(Owner);
            Projectile->LaunchProjectile(Dir, 2000.f);
        }
    }
}
