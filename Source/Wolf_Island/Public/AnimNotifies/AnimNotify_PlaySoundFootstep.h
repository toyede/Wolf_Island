// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify_PlaySound.h"
#include "AnimNotify_PlaySoundFootstep.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UAnimNotify_PlaySoundFootstep : public UAnimNotify_PlaySound
{
	GENERATED_BODY()

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
