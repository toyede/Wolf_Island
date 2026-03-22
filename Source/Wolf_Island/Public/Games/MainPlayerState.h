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
	bool IsReady = false;
	
	UPROPERTY(Replicated)
	ECharacterRole PlayerRole = ECharacterRole::NONE;
	
	UPROPERTY(Replicated)
	TArray<FItemSlot> Items;
	
public:
	AMainPlayerState();
	
	virtual void CopyProperties(APlayerState* PlayerState) override;
	
	virtual void OverrideWith(APlayerState* PlayerState) override;
	
	UFUNCTION(BlueprintCallable)
	void SetItemsData(TArray<FItemSlot> NewItems) { Items = NewItems;};
	
	UFUNCTION(BlueprintCallable)
	void SetPlayerRole(ECharacterRole NewRole) { PlayerRole = NewRole; };
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	ECharacterRole GetPlayerRole() const { return PlayerRole; };
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool GetReady() const { return IsReady; };
	
	UFUNCTION(BlueprintCallable)
	void SetRandomRole();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<FItemSlot> GetItems() const { return Items; };
	
	UFUNCTION(BlueprintCallable)
	void ToggleReady() { IsReady = !IsReady; };
	
	//플레이어의 식별코드를 주는 코드. : 에디터에서 실행하면 테스트용 코드 반환. 실제 환경에선 NetID 반환.
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetPersistantId();
	
	UFUNCTION(BlueprintCallable)
	void PrintItems(float DeltaTime);
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
