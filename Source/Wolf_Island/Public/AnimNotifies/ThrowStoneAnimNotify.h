// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Actors/StoneProjectile.h"
#include "ThrowStoneAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UThrowStoneAnimNotify : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<AStoneProjectile> ProjectileClass;

};
