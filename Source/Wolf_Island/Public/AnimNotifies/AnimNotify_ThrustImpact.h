// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_ThrustImpact.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UAnimNotify_ThrustImpact : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, Category = "Thrust")
	float ThrustRange = 250.f;

	UPROPERTY(EditAnywhere, Category = "Thrust")
	float ThrustForce = 1200.f;

	UPROPERTY(EditAnywhere, Category = "Thrust")
	float UpwardForce = 300.f;

	UPROPERTY(EditAnywhere, Category = "Thrust")
	float ThrustDamage = 20.f;
};
