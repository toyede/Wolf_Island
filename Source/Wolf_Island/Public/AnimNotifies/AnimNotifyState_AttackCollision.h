// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_AttackCollision.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Attack Collision"))
class WOLF_ISLAND_API UAnimNotifyState_AttackCollision : public UAnimNotifyState
{
	GENERATED_BODY()
	
	UAnimNotifyState_AttackCollision(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animtaion, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animtaion, const FAnimNotifyEventReference& EventReference) override;

};
