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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Chest")
	USkeletalMeshComponent* ChestSkeletalMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Chest")
	UStaticMeshComponent* ChestMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Chest")
	UStaticMeshComponent* ChestCoverMesh;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Chest", Replicated)
	class UInventoryComponent* InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest")
	int32 ChestSlotsSize = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chest")
	float ChestWeightCapacity = 2000.0f;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Chest")
	bool IsOccupied = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Widget")
	TSubclassOf<class UChestScreen> ChestWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	USoundBase* ChestOpenSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	USoundBase* ChestCloseSound;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation")
	UAnimationAsset* OpenAnim;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Animation")
	UAnimationAsset* CloseAnim;

	UFUNCTION(BlueprintCallable)
	void OpenChest(AActor* Interactor);
	UFUNCTION(BlueprintCallable)
	void CloseChest();

	virtual void Interact(AActor* Interactor) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(Client, Reliable)
	void Client_OpenChest(AActor* Interactor);
	
	UFUNCTION(Server, Reliable)
	void Server_CloseChest();
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multi_PlayAnimAndSound(UAnimationAsset* Anim, USoundBase* Sound);
	
};
