// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotifies/AnimNotify_ThrustImpact.h"
#include "AI/Enemy_Character/EnemyAIBoss.h"

void UAnimNotify_ThrustImpact::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	AEnemyAIBoss* Boss = Cast<AEnemyAIBoss>(MeshComp->GetOwner());
	if (!Boss) return;

	// 신호만 전달 — 실제 판정/데미지/넉백은 보스가 처리
	Boss->OnThrustImpact();
}
