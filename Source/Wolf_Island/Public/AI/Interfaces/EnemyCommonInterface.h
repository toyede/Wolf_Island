// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnemyCommonInterface.generated.h"

UINTERFACE(MinimalAPI)
class UEnemyCommonInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class WOLF_ISLAND_API IEnemyCommonInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetMovementSpeed(EEnemyState State);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ThrowObject();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Die();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void NormalAttack();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Howling();
};
