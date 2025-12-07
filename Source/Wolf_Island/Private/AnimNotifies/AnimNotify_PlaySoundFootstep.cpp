// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifies/AnimNotify_PlaySoundFootstep.h"

void UAnimNotify_PlaySoundFootstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	
}
