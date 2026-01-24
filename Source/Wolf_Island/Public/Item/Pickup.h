// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDataStruct.h"
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
	UPROPERTY(EditDefaultsOnly, Category = "Item Data")
	UDataTable* ItemDataTable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data", Meta = (ExposeOnSpawn = "true"))
	int32 ItemAmount = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data", Meta = (ExposeOnSpawn = "true"))
	FDataTableRowHandle ItemHandle;
	
	//실제 아이템 정보
	UPROPERTY(ReplicatedUsing = OnRep_ItemReference, EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FItemBaseData ItemReference;
	UPROPERTY(EditAnywhere, Category = "Item Data")
	bool IsPhysics = true;
	
	UFUNCTION(BlueprintCallable, Category = "Item Data")
	void InitializePickUp(const int32 InAmount);

	void InitializeDrop(FItemBaseData ItemToDrop, const int32 InAmount);

	FORCEINLINE FItemBaseData& GetItemData() { return ItemReference; };

	virtual void Interact(AActor* Interactor) override;
	
//에디터에서만 실행
#if WITH_EDITOR
	//에디터에서 월드에 배치된 인스턴스 아이템 코드 바꿀 때마다 업데이트 되게 하는 함수
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:

	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;
	
	//멀티플레이 코드
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	//아이템 데이터 설정 시 액터 비주얼 세팅
	UFUNCTION()
	void OnRep_ItemReference();
};
