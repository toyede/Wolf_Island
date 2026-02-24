// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MainPlayer.h"
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
	ECharacterRole PlayerRole = ECharacterRole::NONE;
	
	UPROPERTY(Replicated)
	TArray<FItemSlot> Items;
	
public:
	
	UFUNCTION(BlueprintCallable)
	void SetItemsData(TArray<FItemSlot> NewItems) { Items = NewItems;};
	
	UFUNCTION(BlueprintCallable)
	void SetPlayerTag(FString NewTag) { PlayerTag = NewTag; };
	
	UFUNCTION(BlueprintCallable)
	void SetPlayerRole(ECharacterRole NewRole) { PlayerRole = NewRole; };
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetPlayerTag() const { return PlayerTag; };
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	ECharacterRole GetPlayerRole() const { return PlayerRole; };
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<FItemSlot> GetItems() const { return Items; };
	
	UFUNCTION(BlueprintCallable)
	void PrintItems(float DeltaTime);
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
