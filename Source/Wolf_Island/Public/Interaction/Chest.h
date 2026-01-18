// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableActor.h"
#include "Chest.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API AChest : public AInteractableActor
{
	GENERATED_BODY()

public:

	AChest();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Chest")
	UStaticMeshComponent* ChestMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Chest")
	UStaticMeshComponent* ChestCoverMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Chest", Replicated)
	class UInventoryComponent* InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest")
	int32 ChestSlotsSize = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest")
	float ChestWeightCapacity = 2000.0f;

	UPROPERTY(Replicated ,VisibleAnywhere, BlueprintReadWrite, Category = "Chest")
	bool IsOccupied = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Chest")
	TSubclassOf<class UChestScreen> ChestWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Chest")
	class USoundBase* ChestSound;

	UFUNCTION(BlueprintCallable)
	void OpenChest();
	UFUNCTION(BlueprintCallable)
	void CloseChest();

	virtual void Interact(AActor* Interactor) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
};
