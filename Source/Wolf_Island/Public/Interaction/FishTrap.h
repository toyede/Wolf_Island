// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableActor.h"
#include "FishTrap.generated.h"

UCLASS()
class WOLF_ISLAND_API AFishTrap : public AInteractableActor
{
	GENERATED_BODY()

public:
	AFishTrap();

	virtual void Interact_Implementation(AActor* Interactor) override;

	UFUNCTION()
	TArray<FName> GetFishTrapRowNames() const;

	virtual void SaveData_Implementation(FActorSaveData& OutData) override;
	virtual void LoadData_Implementation(const FActorSaveData& InData) override;

	UFUNCTION(BlueprintCallable)
	float GetRemainingFishTime() const;
	
	UFUNCTION(BlueprintCallable)
	float GetRemainingBaitTime() const;

	UFUNCTION(Server, Reliable)
	void Server_CloseFishTrap();

	FORCEINLINE class UInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "FishTrap")
	class UInventoryComponent* InventoryComponent;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, Category = "FishTrap")
	class UDataTable* FishTrapItemTable;

	UPROPERTY(EditDefaultsOnly, Category = "FishTrap")
	TSubclassOf<class UUserWidget> FishTrapWidgetClass;

	UPROPERTY(EditAnywhere, Category = "FishTrap", meta = (GetOptions = "GetFishTrapRowNames"))
	TArray<FName> LootTable;

	UPROPERTY(EditAnywhere, Category = "FishTrap", meta = (GetOptions = "GetFishTrapRowNames"))
	TArray<FName> ValidBaitList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FishTrap|Time")
	float FishCatchInterval = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FishTrap|Time")
	float BaitDuration = 60.0f;

	UPROPERTY(Replicated)
	float ServerFishTimerEndTime = 0.0f;

	UPROPERTY(Replicated)
	float ServerBaitTimerEndTime = 0.0f;

private:
	void TryCatchFish();

	FTimerHandle FishTimerHandle;

	UPROPERTY(Replicated)
	bool bIsBaitActive = false;

	UPROPERTY(Replicated)
	float BaitRemainingTime = 0.0f;
};