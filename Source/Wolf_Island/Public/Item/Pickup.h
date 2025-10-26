// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableActor.h"
#include "Pickup.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API APickup : public AInteractableActor
{
	GENERATED_BODY()
public:
	
	APickup();

	UPROPERTY(VisibleAnywhere, Category = "Item Data")
	UStaticMeshComponent* PickupMesh;
	//최대 스택 개수를 초과하면 최대 스택 개수로 초기화됨.
	UPROPERTY(EditInstanceOnly, Category = "Item Data")
	int32 ItemAmount = 1;
	UPROPERTY(EditInstanceOnly, Category = "Item Data")
	FDataTableRowHandle ItemHandle;
	UPROPERTY(VisibleAnywhere, Category = "Item Data")
	class UItemBase* ItemReference;
	UPROPERTY(EditAnywhere, Category = "Item Data")
	bool IsPhysics = true;

	void InitializePickUp(const TSubclassOf<UItemBase> BaseClass, const int32 InAmount);

	void InitializeDrop(UItemBase* ItemToDrop, const int32 InAmount);

	FORCEINLINE UItemBase* GetItemData() { return ItemReference; };

	virtual void Interact(AActor* Interactor) override;

	void PickUp(const AActor* Picker);

//에디터에서만 실행
#if WITH_EDITOR
	//에디터에서 월드에 배치된 인스턴스 아이템 코드 바꿀 때마다 업데이트 되게 하는 함수
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:

	virtual void BeginPlay() override;
};
