// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ItemDataStruct.h"
#include "InventoryComponent.generated.h"

class UItemBase;
DECLARE_MULTICAST_DELEGATE(FOnInventoryUpdated);

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
	FText ItemName;
	//실제 인벤토리에 추가된 아이템 개수
	UPROPERTY(BlueprintReadOnly, Category="Item Add Result")
	int32 ActualAmountAdded;
	//아이템 추가 결과 이넘
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	EItemAddedResult OperationResult;
	//실패 이유
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	EItemFailReason OperationFailReason;
	//결과 메시지
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	FText ResultMessage;
	
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
	float CurrentWeight;
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

	//인벤토리 저장 함수
	UFUNCTION(BlueprintCallable, Category="Inventory Save")
	void SaveInventory();
	//인벤토리 불러오기 함수
	UFUNCTION(BlueprintCallable, Category="Inventory Save")
	void LoadInventory();

	//아이템 추가 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult  HandleAddItem(UItemBase* AddedItem);
	//특정 인덱스에 아이템 넣기
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InsertItemToIndex(int32 Index, UItemBase* Item);
	//인덱스 A,B 아이템 바꾸기
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SwapItems(int32 A, int32 B);
	//특정 인덱스의 아이템 비교
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool CheckSameItemAtIndex(int32 Index, UItemBase* Item);
	//특정 인덱스가 빈 슬롯인 지 확인
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool CheckEmptySlotAtIndex(int32 Index) { return InventoryContents[Index].Item == nullptr; };
	//특정 인덱스의 아이템 반환
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UItemBase* GetItemAtIndex(int32 Index){ return InventoryContents[Index].Item; };
	//해당 ID의 아이템이 있는 슬롯 반환
	FItemSlot* FindSlotByID(FName ItemID);
	//인벤토리에 있는 해당 ID의 아이템 총 개수 반환
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemTotalAmountByID(FName ItemID);
	
	//인벤토리에 있는 아이템과 중복되는 아이템인가 체크(인벤토리에서 불러온 아이템인지)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UItemBase* FindMatchingItem(UItemBase* Item) const;
	//
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UItemBase* FindNextItemByID(UItemBase* Item) const;
	//아이템의 다음 스택 찾기
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UItemBase* FindNextPartialStack(UItemBase* Item) const;
	
	//아이템 단일 삭제 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveSingleInstanceOfItem(UItemBase* Item);
	//아이템 다량 삭제 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 RemoveAmountOfItem(UItemBase* Item, int32 DesiredRemovedAmount);
	//스택 쪼개기
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SplitExistingStack(UItemBase* Item, const int32 AmountToSplit);

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
	bool RepairShip(FRepairRecipeData Recipes);

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
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	float CurrentWeight;

	//인벤토리 슬롯
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TArray<FItemSlot> InventoryContents;

	//단일 아이템 추가 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult HandleNoneStackableItem(UItemBase* AddedItem);
	//스택 가능 아이템 추가 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 HandleStackableItem(UItemBase* AddedItem, int32 RequestedAmount);
	//아이템 무게 계산
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 CalculateWeightAddAmount(UItemBase* Item, int32 Amount);
	//아이템 최대 스택 개수 계산
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 CalculateAmountForFullStackAmount(UItemBase* StackableItem, int32 Amount);
	//아이템 찐 추가 함수
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddNewItem(UItemBase* Item, const int32 Amount);
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
	UItemBase* CreateItemByID(FName ItemID, int32 Amount);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};