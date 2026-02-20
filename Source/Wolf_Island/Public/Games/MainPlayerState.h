// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDataStruct.h"
#include "GameFramework/PlayerState.h"
#include "MainPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API AMainPlayerState : public APlayerState
{
	GENERATED_BODY()
	
	UPROPERTY(Replicated)
	FString PlayerTag;
	
	UPROPERTY(Replicated)
	TArray<FItemSlot> Items;
	
public:
	
	UFUNCTION(BlueprintCallable)
	void SetItemsData(TArray<FItemSlot> NewItems) { Items = NewItems;};
	
	UFUNCTION(BlueprintCallable)
	void SetPlayerTag(FString NewTag) { PlayerTag = NewTag; };
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetPlayerTag() { return PlayerTag; };
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
