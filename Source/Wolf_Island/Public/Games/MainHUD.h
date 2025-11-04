// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MainHUD.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API AMainHUD : public AHUD
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="HUD")
	TSubclassOf<class UPlayerHUD> PlayerHUDClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="HUD")
	TSubclassOf<class UInventory> InventoryWidgetClass;

	UPROPERTY(BlueprintReadWrite, Category="HUD")
	UPlayerHUD* PlayerHUDWidget;
	UPROPERTY(BlueprintReadWrite, Category="HUD")
	UInventory* InventoryWidget;

	bool IsHUDVisible;
	bool IsInventoryVisible;

	UFUNCTION(BlueprintCallable)
	void DisplayHUD();
	UFUNCTION(BlueprintCallable)
	void HideHUD();
	UFUNCTION(BlueprintCallable)
	void ToggleHUD();
	UFUNCTION(BlueprintCallable)
	void DisplayInventory();
	UFUNCTION(BlueprintCallable)
	void HideInventory();
	UFUNCTION(BlueprintCallable)
	void ToggleInventory();

protected:

	virtual void BeginPlay() override;
};
