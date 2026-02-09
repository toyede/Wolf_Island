// Fill out your copyright notice in the Description page of Project Settings.
//TODO: 아이템 데이터를 ItemID로 넘길 지, FItemData로 넘길 지, FSlotData로 넘길 지 결정해야 함.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ItemDataStruct.h"
#include "Interaction/Repair_Actor.h"
#include "InventoryComponent.generated.h"

class APickup;
DECLARE_MULTICAST_DELEGATE(FOnInventoryUpdated);
DECLARE_MULTICAST_DELEGATE(FOnCurrentWeightChanged);

//실패 이유
UENUM(BlueprintType, meta=(ScriptName="ItemAddedResult"))
enum class EItemFailReason : uint8
{
	NoReason UMETA(DisplayName="No Reason"),
	SystemError UMETA(DisplayName="System Error"),
	SlotOverflow UMETA(DisplayName="Slot Overflow"),
	WeightOverflow UMETA(DisplayName="Weight Overflow")
};

//아이템 추가 결과 이넘
UENUM(BlueprintType, meta=(ScriptName="ItemAddedResult"))
enum class EItemAddedResult : uint8
{	
	//아이템 추가 실패
	NoItemAdded UMETA(DisplayName = "Item Adding Failed"),
	//아이템 부분 추가 (일부 남음)
	PartiallyItemAdded UMETA(DisplayName = "Partially Item Added"),	
	//모든 아이템 추가 성공
	AllItemAdded UMETA(DisplayName = "All Item Added"),
};

//아이템 추가 결과 구조체
USTRUCT(BlueprintType)
struct FItemAddResult
{
	GENERATED_BODY()

	FItemAddResult() :
	ActualAmountAdded(0),
	OperationResult(EItemAddedResult::NoItemAdded),
	ResultMessage(FText::GetEmpty())
	{};

	//추가된 아이템 이름
	UPROPERTY(BlueprintReadOnly, Category="Item Add Result")
	FText ItemName = FText::GetEmpty(); // FText 초기화
	//실제 인벤토리에 추가된 아이템 개수
	UPROPERTY(BlueprintReadOnly, Category="Item Add Result")
	int32 ActualAmountAdded = 0; // int32 초기화
	//아이템 추가 결과 이넘
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	EItemAddedResult OperationResult = EItemAddedResult::NoItemAdded; // Enum 초기화
	//실패 이유
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	EItemFailReason OperationFailReason = EItemFailReason::NoReason; // Enum 초기화
	//결과 메시지
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	FText ResultMessage = FText::GetEmpty(); // FText 초기화
	
	//아무것도 추가되지 않음
	static FItemAddResult AddedNone(const FText& ErrorText, EItemFailReason Reason)
	{
		FItemAddResult AddedNoneResult;
		AddedNoneResult.ActualAmountAdded = 0;
		AddedNoneResult.OperationResult = EItemAddedResult::NoItemAdded;
		AddedNoneResult.OperationFailReason = Reason;
		AddedNoneResult.ResultMessage = ErrorText;

		return AddedNoneResult;
	};
	
	//부분 개수만 추가됨
	static FItemAddResult AddedPartial(const FText& ItemName, const int32 PartialAmountAdded, const FText& ErrorText)
	{
		FItemAddResult AddedPartialResult;
		AddedPartialResult.ItemName = ItemName;
		AddedPartialResult.ActualAmountAdded = PartialAmountAdded;
		AddedPartialResult.OperationResult = EItemAddedResult::PartiallyItemAdded;
		AddedPartialResult.OperationFailReason = EItemFailReason::NoReason;
		AddedPartialResult.ResultMessage = ErrorText;

		return AddedPartialResult;
	};
	
	//모두 추가됨
	static FItemAddResult AddedAll(const FText& ItemName, const int32 AmountAdded, const FText& Message)
	{
		FItemAddResult AddedAllResult;
		AddedAllResult.ItemName = ItemName;
		AddedAllResult.ActualAmountAdded = AmountAdded;
		AddedAllResult.OperationResult = EItemAddedResult::AllItemAdded;
		AddedAllResult.OperationFailReason = EItemFailReason::NoReason;
		AddedAllResult.ResultMessage = Message;

		return AddedAllResult;
	};
};

USTRUCT(BlueprintType)
struct FInventorySaveData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	bool IsEmpty = true;
	UPROPERTY(BlueprintReadOnly, Category="Inventory Save Data")
	TArray<FItemSlot> Inventory;
	UPROPERTY(BlueprintReadOnly, Category="Inventory Save Data")
	float CurrentWeight = 0.0f;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WOLF_ISLAND_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();
		
	//인벤토리 업데이트 델리게이트 (아이템 개수 변화 시 호출)
	FOnInventoryUpdated OnInventoryUpdated;
	
	//무게 업데이트 델리게이트 (무게 변화 시 호출)
	FOnCurrentWeightChanged OnCurrentWeightChanged;
	
	//사운드
	UPROPERTY(EditDefaultsOnly, Category="Sound")
	USoundBase* PickUpSound;

	//인벤토리 저장 함수
	UFUNCTION(BlueprintCallable, Category="Inventory Save")
	void SaveInventory();
	//인벤토리 불러오기 함수
	UFUNCTION(BlueprintCallable, Category="Inventory Save")
	void LoadInventory();

	//아이템 추가 함수 - 아이템 먹을 때 Pickup 클래스에서 실행되는 함수
	FItemAddResult HandleAddItem(FItemBaseData AddedItem);
	
	//특정 인덱스에 아이템 넣기
	void InsertItemToIndex(int32 Index, FItemBaseData Item);
	
	//인덱스 A,B 아이템 바꾸기
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SwapItems(int32 A, int32 B);
	
	// 제작 요청 (서버로 전달)
	UFUNCTION(BlueprintCallable, Category = "Craft")
	void Request_MakeItem(FRecipeData Recipe);

	UFUNCTION(Server, Reliable)
	void Server_MakeItem(FRecipeData Recipe);

	// 수리 요청 (서버로 전달)
	UFUNCTION(BlueprintCallable, Category = "Inventory|Repair")
	void Request_RepairShip(FName RecipeName, FRepairRecipeData Recipe, ARepair_Actor* TargetActor);

	UFUNCTION(Server, Reliable)
	void Server_RepairShip(FName RecipeName, FRepairRecipeData Recipe, ARepair_Actor* TargetActor);
	
	//아이템 데이터 불러오기
	FItemData* GetItemData(FName ItemID) const
	{
		if (!IsValid(ItemDataTable))
		{
			//UE_LOG(LogTemp, Warning, TEXT("ItemDataTable is not valid"));
			return nullptr;
		}
		
		if (ItemID.IsNone())
		{
			//UE_LOG(LogTemp, Error, TEXT("ItemID is NAME_None"));
			return nullptr;
		}
		
		if (ItemDataTable)
		{
			return ItemDataTable->FindRow<FItemData>(ItemID, "SearchingItem");
		}
		
		return nullptr;
	}
	
	FItemData* GetItemData(FItemBaseData Item) const
	{
		if (!IsValid(ItemDataTable))
		{
			//UE_LOG(LogTemp, Warning, TEXT("ItemDataTable is not valid"));
			return nullptr;
		}
		
		if (Item.ItemID.IsNone())
		{
			//UE_LOG(LogTemp, Error, TEXT("ItemID is NAME_None"));
			return nullptr;
		}
		
		if (ItemDataTable)
		{
			return ItemDataTable->FindRow<FItemData>(Item.ItemID, "SearchingItem");
		}
		
		return nullptr;
	}
	
	FItemData* GetItemDataAtIndex(int32 Index)
	{
		if (!IsValid(ItemDataTable))
		{
			//UE_LOG(LogTemp, Warning, TEXT("ItemDataTable is not valid"));
			return nullptr;
		}
		
		FItemBaseData Item = GetItemAtIndex(Index);
		
		if (!Item.IsValid())
		{
			//UE_LOG(LogTemp, Error, TEXT("ItemID is NAME_None"));
			return nullptr;
		}
		
		if (ItemDataTable)
		{
			return ItemDataTable->FindRow<FItemData>(Item.ItemID, "SearchingItem");
		}
		
		return nullptr;
	}
	
	//아이템의 무게 반환
	float GetItemSingleWeight(const FName ItemID) const
	{
		float Weight = 0.0f;

		if (const FItemData* ItemData = GetItemData(ItemID))
		{
			//UE_LOG(LogTemp, Warning, TEXT("GetItemData on GetItemSingleWeight : %s"), *ItemID.ToString());
			Weight = ItemData->NumericData.Weight;
		}
		
		return Weight;
	}
	
	float GetItemSingleWeight(const FItemBaseData& Item) const
	{
		float Weight = 0.0f;

		if (const FItemData* ItemData = GetItemData(Item.ItemID))
		{
			//UE_LOG(LogTemp, Warning, TEXT("GetItemData on GetItemSingleWeight : %s | %f"), *Item.ItemID.ToString(), ItemData->NumericData.Weight);
			Weight = ItemData->NumericData.Weight;
		}
		
		return Weight;
	}
	
	//아이템의 최대 스택 무게 반환
	float GetItemStackWeight(const FName ItemID) const
	{
		float Weight = 0.0f;

		if (const FItemData* ItemData = GetItemData(ItemID))
		{
			//UE_LOG(LogTemp, Warning, TEXT("GetItemData on GetItemStackWeight : %s"), *ItemID.ToString());
			Weight = ItemData->NumericData.Weight * ItemData->NumericData.MaxAmount;
		}
		
		return Weight;
	}
	
	float GetItemStackWeight(const FItemBaseData& Item) const
	{
		float Weight = 0.0f;

		if (const FItemData* ItemData = GetItemData(Item.ItemID))
		{
			//UE_LOG(LogTemp, Warning, TEXT("GetItemData on GetItemStackWeight : %s"), *Item.ItemID.ToString());
			Weight = ItemData->NumericData.Weight * ItemData->NumericData.MaxAmount;
		}
		
		return Weight;
	}
	
	//슬롯이 풀스택인지 체크
	bool IsSlotFullStack(const FItemSlot& Slot) const
	{
		if (const FItemData* ItemData = GetItemData(Slot.ItemData.ItemID))
		{
			//UE_LOG(LogTemp, Warning, TEXT("GetItemData on IsSlotFullStack : %s"), *Slot.ItemData.ItemID.ToString());
			return Slot.ItemData.Amount == ItemData->NumericData.MaxAmount;
		}
		
		return false;
	}
	
	//아이템의 최대 스택 개수 반환
	int32 GetItemMaxAmount(const FName ItemID) const
	{
		if (const FItemData* ItemData = GetItemData(ItemID))
		{
			//UE_LOG(LogTemp, Warning, TEXT("GetItemData on GetItemMaxAmount : %s"), *ItemID.ToString());
			return ItemData->NumericData.MaxAmount;
		}
		
		return 0;
	}
	
	int32 GetItemMaxAmount(const FItemBaseData& Item) const
	{
		if (const FItemData* ItemData = GetItemData(Item.ItemID))
		{
			//UE_LOG(LogTemp, Warning, TEXT("GetItemData on GetItemMaxAmount : %s"), *Item.ItemID.ToString());
			return ItemData->NumericData.MaxAmount;
		}
		
		return 0;
	}
	
	//스택 가능한 아이템인 지 체크
	bool IsStackableItem(FName ItemID)
	{
		if (const FItemData* ItemData = GetItemData(ItemID))
		{
			//UE_LOG(LogTemp, Warning, TEXT("GetItemData on IsStackableItem : %s | %d"), *ItemID.ToString(), ItemData->NumericData.IsStackable);
			return ItemData->NumericData.IsStackable;
		} else
		{
			//UE_LOG(LogTemp, Warning, TEXT("No Item Data on [ %s ]"), *ItemID.ToString());

		}
		
		return false;
	}
	
	bool IsStackableItem(const FItemBaseData& Item)
	{
		if (const FItemData* ItemData = GetItemData(Item.ItemID))
		{
			//UE_LOG(LogTemp, Warning, TEXT("GetItemData on IsStackableItem : %s | %d"), *Item.ItemID.ToString(), ItemData->NumericData.IsStackable);
			return ItemData->NumericData.IsStackable;
		} else
		{
			//UE_LOG(LogTemp, Warning, TEXT("No Item Data on [ %s ]"), *Item.ItemID.ToString());
		}
		
		return false;
	}
	
	//사용 가능한 아이템인 지 체크
	bool IsUsableItem(FName ItemID)
	{
		if (const FItemData* ItemData = GetItemData(ItemID))
		{
			return ItemData->NumericData.IsUsable;
		}
		
		return false;
	}
	
	bool IsUsableItem(const FItemBaseData& Item)
	{
		if (const FItemData* ItemData = GetItemData(Item.ItemID))
		{
			return ItemData->NumericData.IsUsable;
		}
		
		return false;
	}
	
	//특정 인덱스의 아이템 개수 반환
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemAmountAtSlot(int32 Index)
	{
		return InventoryContents[Index].ItemData.Amount;
	}
	
	//특정 인덱스의 아이템 비교
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool CheckSameItemAtIndex(int32 Index, FName ItemID);
	
	//특정 인덱스가 빈 슬롯인 지 확인
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool CheckEmptySlotAtIndex(int32 Index)
	{
		return InventoryContents[Index].IsEmpty();
	};
	
	//특정 인덱스의 아이템 데이터 반환
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	FItemBaseData& GetItemAtIndex(int32 Index)
	{
		return InventoryContents[Index].ItemData;
	};
	//해당 ID의 아이템이 있는 슬롯 반환
	FItemSlot* FindSlotByID(FName ItemID);
	
	//인벤토리에 있는 해당 ID의 아이템 총 개수 반환
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemTotalAmountByID(FName ItemID);
	
	//인벤토리에 있는 아이템과 중복되는 아이템인가 체크(인벤토리에서 불러온 아이템인지)
	FItemBaseData* FindMatchingItem(FItemBaseData& Item) const;

	//아이템의 다음 스택 찾기
	FItemBaseData* FindNextPartialStack(const FItemBaseData& Item);
		
	//인벤토리 총 무게 반환
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FORCEINLINE float GetCurrentWeight() const { return CurrentWeight; };
	//인벤토리 무게 용량 반환
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FORCEINLINE float GetWeightCapacity() const { return WeightCapacity; };
	//무게 추가 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void IncreaseCurrentWeight(float Weight);
	//무게 감소 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void DecreaseCurrentWeight(float Weight);
	//인벤토리 용량 반환
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FORCEINLINE int32 GetCapacity() const { return SlotsCapacity; };
	//인벤토리 용량 퍼센트 반환
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	FORCEINLINE float GetWeightPercent() const { return CurrentWeight/WeightCapacity; };
	//인벤토리 아이템 개수 반환
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FORCEINLINE int32 GetItemAmount() const { return InventoryContents.Num(); };
	//인벤토리 슬롯 반환
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FORCEINLINE TArray<FItemSlot> GetInventory() const { return InventoryContents; };

	//인벤토리 슬롯 용량 설정
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FORCEINLINE void SetSlotsCapacity(int32 Capacity) { SlotsCapacity = Capacity; };
	//인벤토리 무게 용량 설정
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FORCEINLINE void SetWeightCapacity(int32 Capacity) { WeightCapacity = Capacity; };

	//레시피 체크
	UFUNCTION(BlueprintCallable, Category = "Craft")
	bool CheckCanMakeRecipe(FRecipeData Recipe);
	bool CheckCanMakeRepair(FRepairRecipeData Recipe);
	//레시피 아이템 제작
	UFUNCTION(BlueprintCallable, Category = "Craft")
	bool MakeItem(FRecipeData Recipe);
	bool RepairShip(FName RecipeName, FRepairRecipeData Recipe, ARepair_Actor* TargetActor);
	
	//아이템 다량 삭제 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 RemoveAmountOfItem(FItemBaseData& Item, int32 DesiredRemovedAmount);


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//아이템 데이터 테이블
	UPROPERTY(EditDefaultsOnly, Category = "Item Data")
	UDataTable* ItemDataTable;

	//인벤토리 슬롯 개수
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	int32 SlotsCapacity;

	//인벤토리 무게 용량
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	float WeightCapacity;

	//현재 인벤토리 무게
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeight, EditDefaultsOnly, Category = "Inventory")
	float CurrentWeight;

	//인벤토리 슬롯
	UPROPERTY(ReplicatedUsing = OnRep_InventoryContents, EditAnywhere, Category = "Inventory")
	TArray<FItemSlot> InventoryContents;

	//단일 아이템 추가 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult HandleNoneStackableItem(FItemBaseData AddedItem);
	
	//스택 가능 아이템 추가 함수
	int32 HandleStackableItem(FItemBaseData& AddedItem, int32 RequestedAmount);
	
	//아이템 무게 계산
	int32 CalculateWeightAddAmount(FItemBaseData& Item, int32 Amount);
	
	//아이템 최대 스택 개수 계산
	int32 CalculateAmountForFullStackAmount(FItemBaseData& StackableItem, int32 Amount);
	
	//아이템 찐 추가 함수
	void AddNewItem(FItemBaseData& Item, const int32 Amount);
	
	//아이템 있는 슬롯 개수 반환
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	FORCEINLINE int32 GetFilledSlots() { return InventoryContents.Num(); };
	
	//배열 max 반환
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	FORCEINLINE int32 GetMaxSlots() { return InventoryContents.Max(); };
	
	//빈 슬롯 반환
	FItemSlot* FindEmptySlot();
	
	//빈 슬롯 개수 반환
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	int32 GetEmptySlotCount();
	
	//해당 ID의 아이템을 개수만큼 인벤토리에서 삭제
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 RemoveItemsByID(FName ItemID, int32 Amount);
	
	//ID와 개수에 따른 아이템 데이터 생성
	FItemBaseData CreateItemByID(FName ItemID, int32 Amount);
	
	//특정 인덱스에 아이템 삽입
	void SetItemAtIndex(FItemBaseData* Item, int32 Index);
	
	//아이템 인벤토리에서 삭제
	void RemoveSingleInstanceOfItem(FItemBaseData& Item);
	
	//슬롯에 아이템 삭제
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItemAtSlot(int32 Index, FItemBaseData Item);
	
	//아이템 수량 감소 함수(슬롯에서)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItemAmountAtSlot(int32 Index, int32 Amount);
	
	//아이템 수량 증가 함수(슬롯에서)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItemAmountAtSlot(int32 Index, int32 Amount);
	
	//서로 다른 인벤토리 간 아이템 스왑
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SwapItemBetweenInventory(
		UInventoryComponent* TargetInventory, int32 TargetIndex, 
		UInventoryComponent* SourceInventory, int32 SourceIndex);
	
	//서로 다른 인벤토리 간 아이템 드롭
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void DropItemBetweenInventory(
		UInventoryComponent* TargetInventory, int32 TargetIndex, 
		UInventoryComponent* SourceInventory, int32 SourceIndex, 
		FItemBaseData Item);
	
	//월드 드롭 아이템 먹기
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void PickupItem(APickup* Item);
	
	//아이템 개수 강제 세팅 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetItemAmountAtSlot(int32 Index, int32 Amount);
	
	//아이템 개수 감소(무게 제외 개수만)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveOnlyItemAmountAtSlot(int32 Index, int32 AddedAmount);
	
	//아이템 개수 증가(무게 제외 개수만)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddOnlyItemAmountAtSlot(int32 Index, int32 AddedAmount);
	
	//아이템 총 무게 계산 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefreshCurrentWeight();
	
	//인벤토리 확인 디버그 함수
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void PrintInventory(float DeltaTime);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//멀티플레이
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	//인벤토리를 건드는 모든 로직들을 추가해야 한다.
	//아이템 추가
	UFUNCTION(Server, Reliable)
	void Server_HandleAddItem(FItemBaseData AddedItem);
	UFUNCTION()
	void Request_HandleAddItem(FItemBaseData AddedItem);
	
	//아이템 삭제(슬롯 인덱스로 해야할 듯)
	UFUNCTION(Server, Reliable)
	void Server_RemoveItemAtSlot(int32 Index, FItemBaseData Item);
	UFUNCTION()
	void Request_RemoveItemAtSlot(int32 Index, FItemBaseData Item);
	
	//아이템 수정(슬롯 인덱스로..?)
	UFUNCTION(Server, Reliable)
	void Server_SetItemAtSlot(int32 Index, FItemBaseData Item);
	UFUNCTION()
	void Request_SetItemAtSlot(int32 Index, FItemBaseData Item);
	
	//아이템 수량 추가(무게 포함)
	UFUNCTION(Server, Reliable)
	void Server_AddItemAmountAtSlot(int32 Index, int32 AddedAmount);
	UFUNCTION()
	void Request_AddItemAmountAtSlot(int32 Index, int32 AddedAmount);
	
	//아이템 수량 추가무게 제외 수량만)-UI 우클릭 드래그용
	UFUNCTION(Server, Reliable)
	void Server_AddOnlyItemAmountAtSlot(int32 Index, int32 AddedAmount);
	UFUNCTION()
	void Request_AddOnlyItemAmountAtSlot(int32 Index, int32 AddedAmount);
	
	//아이템 수량 감소(무게 포함)
	UFUNCTION(Server, Reliable)
	void Server_RemoveItemAmountAtSlot(int32 Index, int32 RemoveAmount);
	UFUNCTION()
	void Request_RemoveItemAmountAtSlot(int32 Index, int32 RemoveAmount);
	
	//아이템 수량 감소(무게 제외 수량만)-UI 우클릭 드래그용
	UFUNCTION(Server, Reliable)
	void Server_RemoveOnlyItemAmountAtSlot(int32 Index, int32 RemoveAmount);
	UFUNCTION()
	void Request_RemoveOnlyItemAmountAtSlot(int32 Index, int32 RemoveAmount);
	
	//아이템 수량 설정 - 특정 개수로 강제 세팅
	UFUNCTION(Server, Reliable)
	void Server_SetItemAmountAtSlot(int32 Index, int32 Amount);
	UFUNCTION()
	void Request_SetItemAmountAtSlot(int32 Index, int32 Amount);
	
	//슬롯 스왑
	UFUNCTION(Server, Reliable)
	void Server_SwapItem(int32 IndexA, int32 IndexB);
	UFUNCTION()
	void Request_SwapItem(int32 IndexA, int32 IndexB);
	
	//다른 인벤토리 간 슬롯 스왑
	UFUNCTION(Server, Reliable)
	void Server_SwapItemBetweenInventory(
		UInventoryComponent* TargetInventory, int32 TargetIndex,
		UInventoryComponent* SourceInventory, int32 SourceIndex);
	UFUNCTION()
	void Request_SwapItemBetweenInventory(
		UInventoryComponent* TargetInventory, int32 TargetIndex, 
		UInventoryComponent* SourceInventory, int32 SourceIndex);
	
	//다른 인벤토리 슬롯에 드롭
	UFUNCTION(Server, Reliable)
	void Server_DropItemBetweenInventory(
		UInventoryComponent* TargetInventory, int32 TargetIndex, 
		UInventoryComponent* SourceInventory, int32 SourceIndex, 
		FItemBaseData Item);
	UFUNCTION()
	void Request_DropItemBetweenInventory(
		UInventoryComponent* TargetInventory, int32 TargetIndex, 
		UInventoryComponent* SourceInventory, int32 SourceIndex, 
		FItemBaseData Item);
	
	//월드 드롭 아이템 먹기
	UFUNCTION(Server, Reliable)
	void Server_PickUp(APickup* Item);
	UFUNCTION()
	void Request_PickUp(APickup* Item);
	
	//서버용 인벤토리 변경 알림
	UFUNCTION()
	void InventoryChanged();
	
	//인벤토리(InventoryContents) 변경 시
	UFUNCTION()
	void OnRep_InventoryContents();
	//인벤토리 무게 변경 시
	UFUNCTION()
	void OnRep_CurrentWeight();
	
	//아이템 획득 결과 전파
	UFUNCTION(Client, Reliable)
	void Client_AddResult(FItemAddResult Result);
	
private:
	
	
};