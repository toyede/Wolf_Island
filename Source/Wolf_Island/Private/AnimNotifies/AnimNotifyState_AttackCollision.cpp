// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifies/AnimNotifyState_AttackCollision.h"
#include "Components/AttackCollisionComponent.h"
#include "AI/Enemy_Character/EnemyAIBase.h"

UAnimNotifyState_AttackCollision::UAnimNotifyState_AttackCollision(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UAnimNotifyState_AttackCollision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animtaion, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animtaion, TotalDuration, EventReference);

	if (const AEnemyAIBase* Owner = Cast<AEnemyAIBase>(MeshComp->GetOwner()))
	{
		Owner->AttackCollisionComponent->TurnOnCollision();
	}
}

void UAnimNotifyState_AttackCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animtaion, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animtaion,  EventReference);

	if (const AEnemyAIBase* Owner = Cast<AEnemyAIBase>(MeshComp->GetOwner()))
	{
		Owner->AttackCollisionComponent->TurnOffCollision();
	}
}

