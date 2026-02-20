// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AttackMeshProvider.generated.h"

UINTERFACE(MinimalAPI)
class UAttackMeshProvider : public UInterface
{
	GENERATED_BODY()
};

class UAttackCollisionComponent;

class WOLF_ISLAND_API IAttackMeshProvider
{
	GENERATED_BODY()

public:
	virtual USkeletalMeshComponent* GetAttackMesh() const = 0;
	virtual UAttackCollisionComponent* GetAttackCollisionComponent() const = 0;
};
