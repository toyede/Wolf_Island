// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifies/AnimNotifyState_AttackCollision.h"
#include "Components/AttackCollisionComponent.h"
#include "AI/Interfaces/AttackMeshProvider.h"

UAnimNotifyState_AttackCollision::UAnimNotifyState_AttackCollision(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UAnimNotifyState_AttackCollision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    if (IAttackMeshProvider* Provider = Cast<IAttackMeshProvider>(MeshComp->GetOwner()))
    {
        if (UAttackCollisionComponent* Collision = Provider->GetAttackCollisionComponent())
        {
            Collision->TurnOnCollision();
        }
    }
}

void UAnimNotifyState_AttackCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    if (IAttackMeshProvider* Provider = Cast<IAttackMeshProvider>(MeshComp->GetOwner()))
    {
        if (UAttackCollisionComponent* Collision = Provider->GetAttackCollisionComponent())
        {
            Collision->TurnOffCollision();
        }
    }
}