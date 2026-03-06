// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Games/SaveInterface.h"
#include "Interaction/InteractableActor.h"
#include "Chest.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FSlotItem
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		meta = (GetOptions = "GetItemRowNames"))
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount;
};

UCLASS()
class WOLF_ISLAND_API AChest : public AInteractableActor
{
	GENERATED_BODY()

public:

	AChest();
	
	UPROPERTY(EditDefaultsOnly, Category = "Item", SaveGame)
	UDataTable* ItemDataTable;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Init Setting")
	TArray<FSlotItem> InitItems;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Chest")
	USkeletalMeshComponent* ChestSkeletalMesh;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Chest")
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
	
	UFUNCTION()
	TArray<FName> GetItemRowNames() const
	{
		TArray<FName> Result;

		if (ItemDataTable)
		{
			Result = ItemDataTable->GetRowNames();
		}

		return Result;
	}
	
	virtual void BeginPlay() override;

	virtual void Interact(AActor* Interactor) override;
	
	virtual void BeginFocus_Implementation() override;
	
	virtual void EndFocus_Implementation() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(Client, Reliable)
	void Client_OpenChest(AActor* Interactor);
	
	UFUNCTION(Server, Reliable)
	void Server_CloseChest();
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multi_PlayAnimAndSound(UAnimationAsset* Anim, USoundBase* Sound);
	
	//저장 관련 코드=================================================================================================
	virtual void SaveData_Implementation(FActorSaveData& OutData) override;
	virtual void LoadData_Implementation(const FActorSaveData& InData) override;
	
};
