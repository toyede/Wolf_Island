// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_ThrustImpact.generated.h"

/**
 * 
 */
/**
 * ThrustImpact 애님 노티파이.
 * 충격파 발생 타이밍에 삽입되며, 보스에게 신호만 전달합니다.
 * 실제 범위 판정 / 데미지 / 넉백 로직은 AEnemyAIBoss::OnThrustImpact() 에서 처리합니다.
 */
UCLASS()
class WOLF_ISLAND_API UAnimNotify_ThrustImpact : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
