// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifies/ThrowStoneAnimNotify.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

void UThrowStoneAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp) return;

    ACharacter* Owner = Cast<ACharacter>(MeshComp->GetOwner());
    if (!Owner || !ProjectileClass) return;

    FVector HandLoc = MeshComp->GetSocketLocation(TEXT("Hand_R"));
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(Owner, 0);
    if (!PlayerPawn) return;

    FVector Dir = (PlayerPawn->GetActorLocation() - HandLoc).GetSafeNormal();
    FRotator Rot = Dir.Rotation();

    UWorld* World = Owner->GetWorld();
    if (World)
    {
        AStoneProjectile* Projectile = World->SpawnActor<AStoneProjectile>(ProjectileClass, HandLoc, Rot);
        if (Projectile)
        {
            Projectile->LaunchProjectile(Dir, 1500.f);
        }
    }
}
