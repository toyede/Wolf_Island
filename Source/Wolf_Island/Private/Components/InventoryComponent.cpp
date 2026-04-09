// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryComponent.h"

#if WITH_EDITOR
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#endif

#include <string>
#include "AdvancedFriendsGameInstance.h"
#include "Character/MainPlayer.h"
#include "Games/MainPlayerState.h"
#include "Item/Pickup.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/PlayerHUD.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	
	// ...
}

// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if WITH_EDITOR
	if (!GEngine)
	{
		FString Owner = this->GetOwner()->GetName();
		FString Name = FString::Printf(TEXT("INVENTORY OWNER [ %s ]"), *Owner);

		UKismetSystemLibrary::PrintString(GetWorld(), Name, true, true, FLinearColor::Green, DeltaTime);
		for (FItemSlot& Slot : InventoryContents)
		{
			FItemBaseData Item = Slot.ItemData;
			FString IsPending = "NULL";
			FString ItemName = "NULL";
			if (Item.ItemID != NAME_None)
			{
				ItemName = GetItemData(Item)->TextData.Name.ToString();
			}
			FString Message = FString::Printf(TEXT("Slot: [ %s ] by %s"), *ItemName, *Owner);

			UKismetSystemLibrary::PrintString(GetWorld(), Message, true, true, FLinearColor::Green, DeltaTime);
		}
	}
#endif
}

void UInventoryComponent::SaveInventory()
{
	UAdvancedFriendsGameInstance* AFGI = Cast<UAdvancedFriendsGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	FInventorySaveData PlayerInventorySaveData;
	PlayerInventorySaveData.IsEmpty = false;
	PlayerInventorySaveData.Inventory = InventoryContents;
	PlayerInventorySaveData.CurrentWeight = CurrentWeight;

	AFGI->PlayerInventory = PlayerInventorySaveData;
}

void UInventoryComponent::LoadInventory()
{
	UAdvancedFriendsGameInstance* AFGI = Cast<UAdvancedFriendsGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	if(AFGI && !AFGI->PlayerInventory.IsEmpty){
		InventoryContents = AFGI->PlayerInventory.Inventory;
		CurrentWeight = AFGI->PlayerInventory.CurrentWeight;
	}
}

FItemSlot* UInventoryComponent::FindSlotByID(FName ItemID)
{
	for (FItemSlot& Slot : InventoryContents)
	{
		if (Slot.IsNotEmpty())
		{
			if (Slot.ItemData.ItemID == ItemID)
			{
				return &Slot;
			}
		}
	}

	return nullptr;
}

int32 UInventoryComponent::GetItemTotalAmountByID(FName ItemID)
{
	int32 count = 0;
    
	for (const FItemSlot& Slot : InventoryContents)
	{
		if (Slot.ItemData.ItemID == ItemID)
		{
			count += Slot.ItemData.Amount;
		}
	}
    
	return count;
}

void UInventoryComponent::SetItemAtIndex(FItemBaseData* Item, int32 Index)
{
	FItemBaseData RemovedItem = GetItemAtIndex(Index);
	
	if (RemovedItem.IsValid())
	{
		//CurrentWeight -= GetItemSingleWeight(RemovedItem) * RemovedItem.Amount;
	}
	if (Item)
	{
		//CurrentWeight += GetItemSingleWeight(*Item) * Item->Amount;
		InventoryContents[Index].ItemData = *Item;
		
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("NO INPUT ITEM"));
		InventoryContents[Index].Clear();
	}
	//OnInventoryUpdated.Broadcast();
}

FItemBaseData* UInventoryComponent::FindMatchingItem(FItemBaseData& Item) const
{
	if (Item.IsValid())
	{
		//인벤토리에 있는 아이템과 같은 아이템이면 그대로 반환
		//if(InventoryContents.Contains(Item)) return Item;
		for (FItemSlot Slot : InventoryContents)
		{
			if (Slot.IsNotEmpty())
			{
				if (Slot.ItemData.ItemID == Item.ItemID)
				{
					return &Item;
				}
			}
		}
	}
	//아니면 Null 반환
	return nullptr;
}

//부분 스택 찾기(풀스택이 아닌 아이템 스택)
FItemBaseData* UInventoryComponent::FindNextPartialStack(const FItemBaseData& Item)
{
	/*if (const TArray<TObjectPtr<UItemBase>>::ElementType* Result =
		InventoryContents.FindByPredicate([&Item](const UItemBase* InventoryItem)
		{
			return InventoryItem->ID == Item->ID && !InventoryItem->IsFullStack();
		})
	)
	{
		return *Result;
	}*/
	for (FItemSlot& Slot : InventoryContents)
	{
		if (Slot.IsNotEmpty() && Slot.ItemData.ItemID == Item.ItemID && !IsSlotFullStack(Slot))
		{
			return &Slot.ItemData;
		}
	}
	
	return nullptr;
}

void UInventoryComponent::RemoveSingleInstanceOfItem(FItemBaseData& Item)
{
	UE_LOG(LogTemp, Warning, TEXT("EXECUTE RSIOI"))
	
	Item.LogData();
	
	for (FItemSlot& Slot : InventoryContents)
	{
		if (Slot.ItemData == Item)
		{
			UE_LOG(LogTemp, Warning, TEXT("RSIOI : FIND SLOT"))
			Slot.Clear();
			break;
		}
	}
	
	//OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::RemoveItemAtSlot(int32 Index, FItemBaseData Item)
{
	
}


int32 UInventoryComponent::RemoveAmountOfItem(FItemBaseData& Item, int32 DesiredRemovedAmount)
{
	UE_LOG(LogTemp, Warning, TEXT("Item Amount : %d | Desired Amount : %d"), Item.Amount, DesiredRemovedAmount);
	//삭제하고 싶은 개수와 실제 아이템 개수 중 작은 값
	const int32 ActualAmountToRemove = FMath::Min(DesiredRemovedAmount, Item.Amount);
	//아이템 개수에서 실제 삭제 개수 빼기
	Item.SetAmount(Item.Amount - ActualAmountToRemove);
	//남은 개수가 없으면 인벤토리에서 삭제
	if (Item.Amount <= 0)
	{
		RemoveSingleInstanceOfItem(Item);
	}
	//무게에서 삭제된 만큼 빼기
	//CurrentWeight -= ActualAmountToRemove * GetItemSingleWeight(Item.ItemID);
	//그 사실을 널리 알리기
	//OnInventoryUpdated.Broadcast();
	//실제 삭제된 개수 반환
	return ActualAmountToRemove;
}

void UInventoryComponent::RemoveItemAmountAtSlot(int32 Index, int32 Amount)
{
	if (!GetItemAtIndex(Index).IsValid()) return;
	
	int32 RemoveAmount = FMath::Min(Amount, InventoryContents[Index].ItemData.Amount);
	
	InventoryContents[Index].ItemData.Amount -= RemoveAmount;
	
	if (InventoryContents[Index].ItemData.Amount <= 0)
	{
		InventoryContents[Index].Clear();
	}
	
	//CurrentWeight -= RemoveAmount * GetItemSingleWeight(InventoryContents[Index].ItemData);
}

void UInventoryComponent::AddItemAmountAtSlot(int32 Index, int32 Amount)
{
	if (!GetItemAtIndex(Index).IsValid() || Amount == 0) return;
	
	int32 AddAmount = FMath::Min(Amount, GetItemMaxAmount(InventoryContents[Index].ItemData)-InventoryContents[Index].ItemData.Amount);

	InventoryContents[Index].ItemData.Amount += AddAmount;
	
	//CurrentWeight += AddAmount * GetItemSingleWeight(InventoryContents[Index].ItemData);
}

void UInventoryComponent::IncreaseCurrentWeight(float Weight)
{
	CurrentWeight += Weight;
	if (CurrentWeight > WeightCapacity) CurrentWeight = WeightCapacity;
	if (CurrentWeight <= 0) CurrentWeight = 0;
}

void UInventoryComponent::DecreaseCurrentWeight(float Weight)
{
	CurrentWeight -= Weight;
	if (CurrentWeight <= 0) CurrentWeight = 0;
	if (CurrentWeight > WeightCapacity) CurrentWeight = WeightCapacity;
}

//아이템 추가 태스크 함수 - 아이템 먹을 때 Pickup 클래스에서 실행되는 함수
FItemAddResult UInventoryComponent::HandleAddItem(FItemBaseData AddedItem)
{
	if (GetOwner())
	{
		//추가할 아이템 개수
		const int32 RequestedAmount = AddedItem.Amount;
		
		UE_LOG(LogTemp, Warning, TEXT("%s is Stackable? : %d"), *AddedItem.ItemID.ToString(), IsStackableItem(AddedItem));

		//여러개 못 드는 아이템일 때 (IsStackable = false 아이템)
		if (!IsStackableItem(AddedItem))
		{
			//단일 아이템 추가 태스크 함수 실행
			return HandleNoneStackableItem(AddedItem);
		}

		//스택 가능 아이템일 때
		//넣은 개수
		const int32 AddedAmount = HandleStackableItem(AddedItem, RequestedAmount);

		//넣은 개수가 추가할 개수랑 같으면
		if (AddedAmount == RequestedAmount)
		{
			//몽땅 추가
			return FItemAddResult::AddedAll(GetItemData(AddedItem)->TextData.Name, RequestedAmount, FText::Format(FText::FromString("Success [ {0} : x{1} ]"), GetItemData(AddedItem)->TextData.Name, RequestedAmount));
		}
		//넣은 개수가 추가할 개수보다 작거나, 넣은 개수가 0 초과면
		if (AddedAmount < RequestedAmount && AddedAmount > 0)
		{
			//부분 추가
			return FItemAddResult::AddedPartial(GetItemData(AddedItem)->TextData.Name, AddedAmount, FText::Format(FText::FromString("Partial Added [ {0} : x{1} ]"), GetItemData(AddedItem)->TextData.Name, AddedAmount));
		}
		//넣은 개수가 0보다 작거나 같다면
		if (AddedAmount <= 0)
		{
			//추가 안해부러
			return FItemAddResult::AddedNone(FText::Format(FText::FromString("Failed Slot Overflow [ {0} : x{1} ]"), GetItemData(AddedItem)->TextData.Name, RequestedAmount), EItemFailReason::SlotOverflow);
		}
	}
	
	return FItemAddResult::AddedNone(FText::FromString(TEXT("Can't Find Owner")),EItemFailReason::SystemError);

}

//단일 아이템 추가 태스크 함수
FItemAddResult UInventoryComponent::HandleNoneStackableItem(FItemBaseData AddedItem)
{
	UE_LOG(LogTemp, Warning, TEXT("Item None Stackable : %s"), *AddedItem.ItemName.ToString());
	if (!AddedItem.IsValid())
	{
		return FItemAddResult::AddedNone(FText::FromString("Item is not VALID"), EItemFailReason::SystemError);	
	}
	
	//추가할 아이템 무게 췤. 음수인 지 음수면 아무것도 안 함
	if (FMath::IsNearlyZero(GetItemSingleWeight(AddedItem)) || GetItemSingleWeight(AddedItem) < 0)
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("[WEIGHT OVERFLOW] [ {0} ]"), GetItemSingleWeight(AddedItem)), EItemFailReason::SystemError);
	}
	//무게 용량 초과면 아무것도 안 함
	if (CurrentWeight + GetItemSingleWeight(AddedItem) > GetWeightCapacity())
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("[WEIGHT OVERFLOW] [ {0} : x{1} ]"),GetItemData(AddedItem)->TextData.Name, GetItemSingleWeight(AddedItem)), EItemFailReason::WeightOverflow);
	}
	//아이템 슬롯 초과 했능가? 초과면 아무것도 안 함
	if (GetEmptySlotCount() <= 0 )
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("[SLOT OVERFLOW] [ {0} : x{1} ]"),GetItemData(AddedItem)->TextData.Name, 1), EItemFailReason::SlotOverflow);
	}

	//이상할 것이 없으면 추가
	AddNewItem(AddedItem, 1);
	return FItemAddResult::AddedAll(GetItemData(AddedItem)->TextData.Name, 1, FText::Format(FText::FromString("[ADDING SUCCESS] [ {0} : x{1} ]"), GetItemData(AddedItem)->TextData.Name, 1));
}

//스택 가능 아이템 추가 태스크 함수
int32 UInventoryComponent::HandleStackableItem(FItemBaseData& AddedItem, int32 RequestedAmount)
{
	//아이템 개수가 0이거나 음수면 0 반환
	if (RequestedAmount <= 0 || FMath::IsNearlyZero(GetItemStackWeight(AddedItem)))
	{
		return 0;
	}

	//인벤토리에 넣을 양
	int32 AmountToDistribute = RequestedAmount;
	//인벤토리에 있는 풀스택이 아닌 같은 아이템 스택
	FItemBaseData* ExistingItemStack = FindNextPartialStack(AddedItem);
	
	//인벤토리에 풀스택이 아닌 같은 아이템이 없을 때까지 반복
	while (ExistingItemStack)
	{
		UE_LOG(LogTemp, Warning, TEXT("Enter While"));
		//풀스택까지 남은 개수
		const int32 AmountToMakeFullStack = CalculateAmountForFullStackAmount(*ExistingItemStack, AmountToDistribute);
		//무게 초과하지 않는 개수
		const int32 WeightLimitAddAmount = CalculateWeightAddAmount(*ExistingItemStack, AmountToMakeFullStack);

		//무게 초과하지 않는 개수가 0 초과면
		if (WeightLimitAddAmount > 0)
		{
			//인벤토리에 있던 아이템 개수 추가
			ExistingItemStack->SetAmount(ExistingItemStack->Amount + WeightLimitAddAmount);
			UE_LOG(LogTemp, Warning, TEXT("ORIGIN WEIGHT : %f"), CurrentWeight);
			//개수 추가한 만큼 인벤토리 무게 추가
			float v = GetItemSingleWeight(*ExistingItemStack) * WeightLimitAddAmount;
			UE_LOG(LogTemp, Warning, TEXT("ADDED WEIGHT : %f"), v);
			CurrentWeight += v;
			UE_LOG(LogTemp, Warning, TEXT("AFTER WEIGHT : %f"), CurrentWeight);

			//넣을 양에서 넣은 양 빼기
			AmountToDistribute -= WeightLimitAddAmount;
			//넣을 아이템 개수 수정
			AddedItem.SetAmount(AmountToDistribute);

			//무게 용량 초과면
			if (CurrentWeight >= WeightCapacity)
			{
				//OnInventoryUpdated.Broadcast();
				//요청한 개수에서 넣을 개수 빼고 반환
				UE_LOG(LogTemp, Warning, TEXT("OVERWEIGHT"));
				return RequestedAmount - AmountToDistribute;
			}
		}
		//무게 초과하지 않는 개수가 0개 이하면
		else if (WeightLimitAddAmount <= 0)
		{
			//넣을 개수가 요청한 개수랑 다르다면
			if (AmountToDistribute != RequestedAmount)
			{
				//OnInventoryUpdated.Broadcast();
				//요청한 개수에서 넣을 개수 빼고 반환
				UE_LOG(LogTemp, Warning, TEXT("Distribute Request Not Same"));
				return RequestedAmount - AmountToDistribute;
			}

			//넣을 개수가 요청한 개수랑 같으면 0반환
			return 0;
		}
		
		//넣을 개수가 0이하면
		if (AmountToDistribute <= 0)
		{
			//OnInventoryUpdated.Broadcast();
			//요청한 개수 반환
			UE_LOG(LogTemp, Warning, TEXT("Distribute under 0"));
			return RequestedAmount;
		}

		//다음 스택 찾기
		ExistingItemStack = FindNextPartialStack(AddedItem);
	}
	
	//남은 아이템 슬롯이 있는가
	if (GetEmptySlotCount() > 0)
	{
		//무게 고려 최대 넣을 수 있는 수량
		const int32 WeightLimitAddAmount = CalculateWeightAddAmount(AddedItem, AmountToDistribute);

		//무게 초과하지 않는 개수가 0개 초과면
		if (WeightLimitAddAmount > 0)
		{
			//무게 초과하지 않는 개수가 넣으려는 개수보다 적으면
			if (WeightLimitAddAmount < AmountToDistribute)
			{
				//넣으려는 개수에서 무게 초과하지 않는 개수 빼기
				AmountToDistribute -= WeightLimitAddAmount;
				//넣으려는 아이템 개수 수정
				AddedItem.SetAmount(AmountToDistribute);

				//아이템 복사본을 무게 초과하지 않는 개수만큼 추가
				AddNewItem(AddedItem, WeightLimitAddAmount);
				//요청한 개수에서 넣으려는 개수 빼고 반환
				UE_LOG(LogTemp, Warning, TEXT("WeightLimitAmount over 0"));
				return RequestedAmount - AmountToDistribute;
			}
			//아니면 넣으려는 수 만큼 아이템 추가
			UE_LOG(LogTemp, Warning, TEXT("Amount to Distribute : %d"), AmountToDistribute);
			AddNewItem(AddedItem, AmountToDistribute);
			return RequestedAmount;
		}
	}

	//OnInventoryUpdated.Broadcast();
	//요청한 개수에서 넣은 개수 빼고 반환
	UE_LOG(LogTemp, Warning, TEXT("REQUEST - DISTRIBUTE"));
	return RequestedAmount - AmountToDistribute;
}

//무게에 따른 추가 개수 계산 함수
int32 UInventoryComponent::CalculateWeightAddAmount(FItemBaseData& Item, int32 Amount)
{
	//남은 용량에서 더 추가할 수 있는 개수
	const int32 WeightMaxAddAmount = 
		FMath::FloorToInt((GetWeightCapacity() - CurrentWeight) / GetItemSingleWeight(Item));
	UE_LOG(LogTemp, Warning, TEXT("Addable Weight Max Amount : %d"),WeightMaxAddAmount);
	//추가할 아이템이 가능한 개수보다 적으면 추가할 개수 반환
	if (WeightMaxAddAmount >= Amount)
	{
		return Amount;
	}
	//아니면 가능한 개수 반환
	return WeightMaxAddAmount;
}

//스택 아이템 추가 개수 계산 함수
int32 UInventoryComponent::CalculateAmountForFullStackAmount(FItemBaseData& StackableItem, int32 Amount)
{
	//풀 스택 만들기 위해 남은 개수
	const int32 AmountToMakeFullStack = GetItemMaxAmount(StackableItem) - StackableItem.Amount;

	//둘 중 더 작은 값 반환
	return FMath::Min(Amount, AmountToMakeFullStack);
}

//실직적 아이템 추가 함수
void UInventoryComponent::AddNewItem(FItemBaseData& Item, const int32 Amount)
{
	FItemBaseData NewItem = Item;

	NewItem.SetAmount(Amount);

	//인벤토리에 추가
	FindEmptySlot()->ItemData = NewItem;
	
	//무게 추가
	//CurrentWeight += GetItemSingleWeight(NewItem) * Amount;
	//그 사실을 널리 알리기
	//OnInventoryUpdated.Broadcast();
}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// ...
	SetIsReplicated(true);
}


FItemSlot* UInventoryComponent::FindEmptySlot()
{
	for (FItemSlot& Slot : InventoryContents)
	{
		if (Slot.IsEmpty())
		{
			return &Slot;
		}
	}
	
	return nullptr;
}

int32 UInventoryComponent::GetEmptySlotCount()
{
	int32 count = 0;
	
	for (FItemSlot& Slot : InventoryContents)
	{
		if (Slot.IsEmpty())
		{
			count++;
		}
	}

	return count;
}

int32 UInventoryComponent::RemoveItemsByID(FName ItemID, int32 Amount)
{
	int32 DesiredRemoveAmount = Amount;
    
	for (FItemSlot& Slot : InventoryContents)
	{
		if (Slot.ItemData.ItemID == ItemID)
		{
			int32 RemoveAmount = FMath::Min(DesiredRemoveAmount, Slot.ItemData.Amount);
            
			Slot.ItemData.Amount -= RemoveAmount;
			DesiredRemoveAmount -= RemoveAmount;

			if (Slot.ItemData.Amount <= 0) 
			{
				Slot.Clear(); 
			}

			if (DesiredRemoveAmount <= 0) break;
		}
	}
    
	return Amount - DesiredRemoveAmount;
}

FItemBaseData UInventoryComponent::CreateItemByID(FName ItemID, int32 Amount)
{
	//데이터 데이블에서 아이템 데이터 가져오기
	if (!ItemDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemDataTable is null"));
		return FItemBaseData();
	}

	FItemData* ItemData = ItemDataTable->FindRow<FItemData>(ItemID, TEXT("CreateItemByID"));
	
	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemData is null"));
		return FItemBaseData();
	}
	
	//아이템 데이터 생성
	FItemBaseData NewItem = FItemBaseData();

	//데이터 테이블에서 아이템 데이터 삽입
	NewItem.ItemID = ItemData->ID;
	NewItem.SetAmount(Amount);

	if (ItemData)
	{
		const bool bIsEquipment = (ItemData->Type == EItemType::EQUIPMENT);

		if (bIsEquipment)
		{
			NewItem.CurrentDurability = ItemData->NumericData.Durability;
		}
		else
		{
			NewItem.CurrentDurability = 0.0f;
		}
	}

	//아이템 데이터 반환
	return NewItem;
}

void UInventoryComponent::PrintInventory(float DeltaTime)
{
	FString Name = GetOwner()->GetName();
	
	UKismetSystemLibrary::PrintString(GetWorld(), Name, true, true, FLinearColor::Green, DeltaTime);
	
	for (FItemSlot& Slot : InventoryContents)
	{
		FString Item = "[ "+ Slot.ItemData.ItemName.ToString() + " | " + FString::FromInt(Slot.ItemData.Amount) + "EA ]";
		UKismetSystemLibrary::PrintString(GetWorld(), Item, true, true, FLinearColor::Green, DeltaTime);
	}	
}

bool UInventoryComponent::ConsumeRecipeIngredients(const FRecipeData& Recipe)
{
	if (!GetOwner()->HasAuthority()) return false;

	if (!CheckCanMakeRecipe(Recipe))
	{
		return false;
	}

	TMap<FName, int32> Ingredients = Recipe.GetIngredients();
	for (const auto& Pair : Ingredients)
	{
		RemoveItemsByID(Pair.Key, Pair.Value);
	}

	InventoryChanged();
    
	return true;
}

bool UInventoryComponent::CheckCanMakeRecipe(FRecipeData Recipe)
{
	bool pass1 = false, pass2 = false, pass3 = false;

	//재료 아이템이 비어있으면 그 칸은 패스
	if (Recipe.Ingredient1ID.IsNone()) pass1 = true;
	if (Recipe.Ingredient2ID.IsNone()) pass2 = true;
	if (Recipe.Ingredient3ID.IsNone()) pass3 = true;

	//첫번째 재료 인벤토리에서 체크
	//레시피 개수보다 인벤토리에 아이템 개수가 같거나 많으면 패스
	if (Recipe.Ingredient1Amount <= GetItemTotalAmountByID(Recipe.Ingredient1ID)) pass1 = true;
	//두번째 재료 인벤토리에서 체크
	//레시피 개수보다 인벤토리에 아이템 개수가 같거나 많으면 패스
	if (Recipe.Ingredient2Amount <= GetItemTotalAmountByID(Recipe.Ingredient2ID)) pass2 = true;
	//세번째 재료 인벤토리에서 체크
	//레시피 개수보다 인벤토리에 아이템 개수가 같거나 많으면 패스
	if (Recipe.Ingredient3Amount <= GetItemTotalAmountByID(Recipe.Ingredient3ID)) pass3 = true;	
	
	return pass1 && pass2 && pass3;
}

bool UInventoryComponent::CheckCanMakeRepair(FRepairRecipeData Recipe)
{
	bool pass1 = false, pass2 = false, pass3 = false;

	//재료 아이템이 비어있으면 그 칸은 패스
	if (Recipe.Ingredient1ID.IsNone()) pass1 = true;
	if (Recipe.Ingredient2ID.IsNone()) pass2 = true;
	if (Recipe.Ingredient3ID.IsNone()) pass3 = true;

	//첫번째 재료 인벤토리에서 체크
	//레시피 개수보다 인벤토리에 아이템 개수가 같거나 많으면 패스
	if (Recipe.Ingredient1Amount <= GetItemTotalAmountByID(Recipe.Ingredient1ID)) pass1 = true;
	//두번째 재료 인벤토리에서 체크
	//레시피 개수보다 인벤토리에 아이템 개수가 같거나 많으면 패스
	if (Recipe.Ingredient2Amount <= GetItemTotalAmountByID(Recipe.Ingredient2ID)) pass2 = true;
	//세번째 재료 인벤토리에서 체크
	//레시피 개수보다 인벤토리에 아이템 개수가 같거나 많으면 패스
	if (Recipe.Ingredient3Amount <= GetItemTotalAmountByID(Recipe.Ingredient3ID)) pass3 = true;	
	
	return pass1 && pass2 && pass3;
}

bool UInventoryComponent::MakeItem(FRecipeData Recipe)
{
	UE_LOG(LogTemp, Warning, TEXT("Make Item Execute."));
	//레시피 다시한번 체크
	if (CheckCanMakeRecipe(Recipe))
	{
		//레시피 개수별로 아이템 삭제
		RemoveItemsByID(Recipe.Ingredient1ID, Recipe.Ingredient1Amount);
		RemoveItemsByID(Recipe.Ingredient2ID, Recipe.Ingredient2Amount);
		RemoveItemsByID(Recipe.Ingredient3ID, Recipe.Ingredient3Amount);

		//결과물 아이템 생성
		FItemBaseData ResultItem = CreateItemByID(Recipe.ResultID, Recipe.ResultAmount);
		
		if (ResultItem.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Result Item Created."));
			//빈 슬롯이 없으면 바닥에 떨구기
			if (GetEmptySlotCount() <= 0)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = GetOwner();
				SpawnParams.bNoFail = true;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				const FVector SpawnLocation(SpawnParams.Owner->GetActorLocation() + (SpawnParams.Owner->GetActorForwardVector() * 50.0f));
				const FTransform SpawnTransform(SpawnParams.Owner->GetActorRotation(), SpawnLocation);
		
				APickup* Pickup = GetWorld()->SpawnActor<APickup>(APickup::StaticClass(), SpawnTransform, SpawnParams);
				Pickup->InitializeDrop(ResultItem, ResultItem.Amount);
				
			}
			//있으면 추가
			else
			{
				AddNewItem(ResultItem, ResultItem.Amount);
			}
			InventoryChanged();
			return true;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Recipe Check Failed."));
	
	return false;
}

bool UInventoryComponent::RepairShip(FName RecipeName, FRepairRecipeData Recipe, ARepair_Actor* TargetActor)
{
	if (!TargetActor) return false;

	// 인벤토리에 재료가 충분한지 체크
	if (CheckCanMakeRepair(Recipe))
	{
		// 재료 소모
		RemoveItemsByID(Recipe.Ingredient1ID, Recipe.Ingredient1Amount);
		RemoveItemsByID(Recipe.Ingredient2ID, Recipe.Ingredient2Amount);
		RemoveItemsByID(Recipe.Ingredient3ID, Recipe.Ingredient3Amount);
		RemoveItemsByID(Recipe.Ingredient4ID, Recipe.Ingredient4Amount);

		TargetActor->MarkRecipeAsComplete(RecipeName);
        
		TargetActor->CompleteRepair();

		InventoryChanged();
		return true;
	}
    
	UE_LOG(LogTemp, Warning, TEXT("Repair Failed: Not enough ingredients."));
	return false;
}

void UInventoryComponent::InsertItemToIndex(int32 Index, FItemBaseData Item)
{
	if (InventoryContents.IsValidIndex(Index))
	{
		InventoryContents[Index].ItemData = Item;
		//CurrentWeight += GetItemData(Item)->NumericData.Weight * Item.Amount;
	}
}

//A가 옮기는 슬롯, B가 가만히 있는 슬롯
void UInventoryComponent::SwapItems(int32 A, int32 B)
{
	FItemSlot& SlotA = InventoryContents[A];
	FItemSlot& SlotB = InventoryContents[B];
	
	//서로 다른 아이템이거나 하나라도 빈 슬롯이면 자리 교환
	if ( SlotA.IsEmpty() || SlotB.IsEmpty() || SlotA.ItemData.ItemID != SlotB.ItemData.ItemID)
	{
		Swap(SlotA, SlotB);
	}
	//같은 아이템이면 스택 확인
	else
	{
		// 같은 아이템이면 스택 합치기
		//분배할 총 개수
		int32 TotalAmount = SlotA.ItemData.Amount + SlotB.ItemData.Amount;
		//최대 스택 개수
		int32 MaxStack = GetItemMaxAmount(SlotB.ItemData);
		
		//옮길(B) 슬롯에 총 개수와 최대 스택 개수 중 작은 것 할당
		SlotB.ItemData.Amount = FMath::Min(TotalAmount, MaxStack);
		//옮기는(A) 슬롯에 총 개수 - 옮길(B) 슬롯 개수 할당
		SlotA.ItemData.Amount = TotalAmount - SlotB.ItemData.Amount;

		if (SlotA.ItemData.Amount <= 0){
			SlotA.Clear();
		}
	}
	
	//OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::Request_MakeItem(FRecipeData Recipe)
{
	if (GetOwner()->HasAuthority()) {
		MakeItem(Recipe);
	} else {
		Server_MakeItem(Recipe);
	}
}

void UInventoryComponent::Request_RepairShip(FName RecipeName, FRepairRecipeData Recipe, ARepair_Actor* TargetActor)
{
	if (GetOwner()->HasAuthority())
	{
		RepairShip(RecipeName, Recipe, TargetActor);
	}
	else
	{
		Server_RepairShip(RecipeName, Recipe, TargetActor);
	}
}

void UInventoryComponent::Server_RepairShip_Implementation(FName RecipeName, FRepairRecipeData Recipe, ARepair_Actor* TargetActor)
{
	RepairShip(RecipeName, Recipe, TargetActor);
}

void UInventoryComponent::Server_MakeItem_Implementation(FRecipeData Recipe)
{
	MakeItem(Recipe);
}

void UInventoryComponent::SwapItemBetweenInventory(UInventoryComponent* TargetInventory, int32 TargetIndex,
                                                   UInventoryComponent* SourceInventory, int32 SourceIndex)
{
	//외부에서 온 슬롯
	FItemSlot& OriginSlot = SourceInventory->InventoryContents[SourceIndex];
	//드롭 받는 슬롯
	FItemSlot& TargetSlot = TargetInventory->InventoryContents[TargetIndex];
	
	//서로 다른 아이템이거나 하나라도 빈 슬롯이면 그냥자리 교환
	if (OriginSlot.IsEmpty() || TargetSlot.IsEmpty() || OriginSlot.ItemData.ItemID != TargetSlot.ItemData.ItemID)
	{		
		Swap(OriginSlot, TargetSlot);
	}
	//같은 아이템이면 스택 확인
	else
	{
		// 같은 아이템이면 스택 합치기
		//분배할 총 개수
		int32 TotalAmount = OriginSlot.ItemData.Amount + TargetSlot.ItemData.Amount;
		//최대 스택 개수
		int32 MaxStack = GetItemMaxAmount(OriginSlot.ItemData);
		
		//드롭 받는 슬롯에 총 개수와 최대 스택 개수 중 작은 것 할당
		TargetSlot.ItemData.Amount = FMath::Min(TotalAmount, MaxStack);
		//드래그 가져온 슬롯에 총 개수 - 가져간 슬롯 개수 할당
		OriginSlot.ItemData.Amount = TotalAmount - TargetSlot.ItemData.Amount;
		
		if (OriginSlot.ItemData.Amount <= 0){
			OriginSlot.Clear();
		}
	}
}

void UInventoryComponent::DropItemBetweenInventory(UInventoryComponent* TargetInventory, int32 TargetIndex,
	UInventoryComponent* SourceInventory, int32 SourceIndex, FItemBaseData Item)
{
	//드래그 가져온 슬롯
	FItemSlot& OriginSlot = SourceInventory->InventoryContents[SourceIndex];
	//드롭 받는 슬롯
	FItemSlot& TargetSlot = TargetInventory->InventoryContents[TargetIndex];
	
	//빈 슬롯이면 삽입
	if (TargetSlot.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("DIBI: EMPTY SLOT INSERT"));
		TargetInventory->InsertItemToIndex(TargetIndex, Item);
		
		return;
	}
	//다른 아이템이면 원상복구
	if (TargetSlot.ItemData.ItemID != Item.ItemID)
	{
		UE_LOG(LogTemp, Warning, TEXT("DIBI: BACK TO NORMAL"));
		SourceInventory->AddItemAmountAtSlot(SourceIndex, Item.Amount);
		
		return;
	}
	
	//같은 아이템이면 연산 후 정리
	//분배할 총 개수
	int32 TotalAmount = OriginSlot.ItemData.Amount + Item.Amount;
	//최대 스택 개수
	int32 MaxStack = GetItemMaxAmount(TargetSlot.ItemData.ItemID);

	//드롭 받는 슬롯에 총 개수와 최대 스택 개수 중 작은 것 할당
	TargetSlot.ItemData.Amount = FMath::Min(TotalAmount, MaxStack);
	//드래그 가져온 슬롯에 총 개수 - 드롭 받는 슬롯 개수 할당
	OriginSlot.ItemData.Amount = TotalAmount - TargetSlot.ItemData.Amount;
	
	if (OriginSlot.ItemData.Amount <= 0)
	{
		OriginSlot.Clear();
	}
}

void UInventoryComponent::PickupItem(APickup* Item)
{
	if (!Item->IsPendingKillPending())
	{
		const FItemAddResult AddResult = HandleAddItem(Item->ItemReference);
		Client_AddResult(AddResult);
		
		switch (AddResult.OperationResult)
		{
			//아이템 추가 안됨
		case EItemAddedResult::NoItemAdded:
			//디버깅 결과 메시지
			UE_LOG(LogTemp, Warning, TEXT("Didn't Eat Item"));
			break;
			//아이템 부분만 먹음
		case EItemAddedResult::PartiallyItemAdded:
			//디버깅 결과 메시지
			UE_LOG(LogTemp, Warning, TEXT("Remain Some"));
			break;
			//아이템 싹싹김치
		case EItemAddedResult::AllItemAdded:
			//디버깅 결과 메시지
			UE_LOG(LogTemp, Warning, TEXT("Got All Item"));
			Item->Destroy();
			break;
		}
	}
}

void UInventoryComponent::SetItemAmountAtSlot(int32 Index, int32 Amount)
{
	//개수 강제 세팅 개수로 수정
	InventoryContents[Index].ItemData.Amount = Amount;
}

void UInventoryComponent::RemoveOnlyItemAmountAtSlot(int32 Index, int32 AddedAmount)
{
	if (!GetItemAtIndex(Index).IsValid()) return;
	
	int32 RemoveAmount = FMath::Min(AddedAmount, InventoryContents[Index].ItemData.Amount);
	
	InventoryContents[Index].ItemData.Amount -= RemoveAmount;
	
	if (InventoryContents[Index].ItemData.Amount <= 0)
	{
		InventoryContents[Index].Clear();
	}
}

void UInventoryComponent::AddOnlyItemAmountAtSlot(int32 Index, int32 AddedAmount)
{
	if (!GetItemAtIndex(Index).IsValid() || AddedAmount == 0) return;
	
	int32 AddAmount = FMath::Min(AddedAmount, GetItemMaxAmount(InventoryContents[Index].ItemData)-InventoryContents[Index].ItemData.Amount);

	InventoryContents[Index].ItemData.Amount += AddAmount;
	
}

void UInventoryComponent::RefreshCurrentWeight()
{
	CurrentWeight = 0;
	
	for (FItemSlot& ItemSlot : InventoryContents)
	{
		if (ItemSlot.ItemData.IsValid())
		{
			CurrentWeight += ItemSlot.ItemData.Amount * GetItemSingleWeight(ItemSlot.ItemData);
		}
	}
	
	OnCurrentWeightChanged.Broadcast();
}

bool UInventoryComponent::CheckSameItemAtIndex(int32 Index, FName ItemID)
{
	if (InventoryContents[Index].IsNotEmpty())
	{
		return InventoryContents[Index].ItemData.ItemID == ItemID;
	}
	return false;
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, CurrentWeight);
	DOREPLIFETIME(UInventoryComponent, InventoryContents);
}

void UInventoryComponent::Request_HandleAddItem(FItemBaseData AddedItem)
{
	if (GetOwner()->HasAuthority())
	{
		HandleAddItem(AddedItem);
		InventoryChanged();
	} else
	{
		Server_HandleAddItem(AddedItem);
	}
}

void UInventoryComponent::Request_RemoveItemAtSlot(int32 Index, FItemBaseData Item)
{
	if (GetOwner()->HasAuthority())
	{
		RemoveItemAtSlot(Index, Item);
		InventoryChanged();
	} else
	{
		Server_RemoveItemAtSlot(Index, Item);
	}
}

void UInventoryComponent::Request_SetItemAtSlot(int32 Index, FItemBaseData Item)
{
	if (GetOwner()->HasAuthority())
	{
		InsertItemToIndex(Index, Item);
		InventoryChanged();
	} else
	{
		Server_SetItemAtSlot(Index, Item);
	}
}

void UInventoryComponent::Request_AddItemAmountAtSlot(int32 Index, int32 AddedAmount)
{
	if (GetOwner()->HasAuthority())
	{
		AddItemAmountAtSlot(Index, AddedAmount);
		InventoryChanged();
	} else
	{
		Server_AddItemAmountAtSlot(Index, AddedAmount);
	}
}

void UInventoryComponent::Server_AddOnlyItemAmountAtSlot_Implementation(int32 Index, int32 AddedAmount)
{
	AddOnlyItemAmountAtSlot(Index, AddedAmount);
	InventoryChanged();
}

void UInventoryComponent::Request_AddOnlyItemAmountAtSlot(int32 Index, int32 AddedAmount)
{
	if (GetOwner()->HasAuthority())
	{
		AddOnlyItemAmountAtSlot(Index, AddedAmount);
		InventoryChanged();
	} else
	{
		Server_AddOnlyItemAmountAtSlot(Index, AddedAmount);
	}
}

void UInventoryComponent::Request_RemoveItemAmountAtSlot(int32 Index, int32 RemoveAmount)
{
	if (GetOwner()->HasAuthority())
	{
		RemoveItemAmountAtSlot(Index, RemoveAmount);
		InventoryChanged();
	} else
	{
		Server_RemoveItemAmountAtSlot(Index, RemoveAmount);
	}
}

void UInventoryComponent::Server_RemoveOnlyItemAmountAtSlot_Implementation(int32 Index, int32 RemoveAmount)
{
	RemoveOnlyItemAmountAtSlot(Index, RemoveAmount);
	InventoryChanged();
}

void UInventoryComponent::Request_RemoveOnlyItemAmountAtSlot(int32 Index, int32 RemoveAmount)
{
	if (GetOwner()->HasAuthority())
	{
		RemoveOnlyItemAmountAtSlot(Index, RemoveAmount);
		InventoryChanged();
	} else
	{
		Server_RemoveOnlyItemAmountAtSlot(Index, RemoveAmount);
	}
}

void UInventoryComponent::Server_SetItemAmountAtSlot_Implementation(int32 Index, int32 Amount)
{
	SetItemAmountAtSlot(Index, Amount);
	InventoryChanged();
}

void UInventoryComponent::Request_SetItemAmountAtSlot(int32 Index, int32 Amount)
{
	if (GetOwner()->HasAuthority())
	{
		SetItemAmountAtSlot(Index, Amount);
		InventoryChanged();
	} else
	{
		Server_SetItemAmountAtSlot(Index, Amount);
	}
}

void UInventoryComponent::Request_SwapItem(int32 IndexA, int32 IndexB)
{
	if (GetOwner()->HasAuthority())
	{
		SwapItems(IndexA, IndexB);
		InventoryChanged();
	} else
	{
		Server_SwapItem(IndexA, IndexB);
	}
}

void UInventoryComponent::Request_SwapItemBetweenInventory(UInventoryComponent* TargetInventory, int32 TargetIndex,
	UInventoryComponent* SourceInventory, int32 SourceIndex)
{
	if (GetOwner()->HasAuthority())
	{
		SwapItemBetweenInventory(TargetInventory, TargetIndex, SourceInventory, SourceIndex);
		SourceInventory->InventoryChanged();
		TargetInventory->InventoryChanged();
	} else
	{
		Server_SwapItemBetweenInventory(TargetInventory, TargetIndex, SourceInventory, SourceIndex);
	}
}

void UInventoryComponent::Request_DropItemBetweenInventory(UInventoryComponent* TargetInventory, int32 TargetIndex,
UInventoryComponent* SourceInventory, int32 SourceIndex, FItemBaseData Item)
{
	if (GetOwner()->HasAuthority() || TargetInventory->GetOwner()->HasAuthority())
	{
		DropItemBetweenInventory(TargetInventory, TargetIndex, SourceInventory, SourceIndex, Item);
		SourceInventory->InventoryChanged();
		TargetInventory->InventoryChanged();
	} else
	{
		Server_DropItemBetweenInventory(TargetInventory, TargetIndex, SourceInventory, SourceIndex, Item);
	}
}

void UInventoryComponent::Request_PickUp(APickup* Item)
{
	if (GetOwner()->HasAuthority())
	{
		PickupItem(Item);
		InventoryChanged();
	} else
	{
		Server_PickUp(Item);
	}
}

void UInventoryComponent::InventoryChanged()
{
	//UE_LOG(LogTemp, Warning, TEXT("%hs INVENTORY CHANGED"), GetOwner()->HasAuthority() ? "SERVER" : "CLIENT");
	RefreshCurrentWeight();
	OnInventoryUpdated.Broadcast();
	
	//인벤토리 업데이트 시 플레이어 스테이트에 인벤토리 정보 저장
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;
	
	AController* Controller = OwnerPawn->GetController();
	if (!Controller) return;
	
	AMainPlayerState* PS = Cast<AMainPlayerState>(OwnerPawn->GetController()->PlayerState);
	if (!PS) return;
	
	PS->SetItemsData(GetInventory());
}

void UInventoryComponent::Server_HandleAddItem_Implementation(FItemBaseData AddedItem)
{
	HandleAddItem(AddedItem);
	InventoryChanged();
}

void UInventoryComponent::Server_RemoveItemAtSlot_Implementation(int32 Index, FItemBaseData Item)
{
	RemoveItemAtSlot(Index, Item);
	InventoryChanged();
}

void UInventoryComponent::Server_SetItemAtSlot_Implementation(int32 Index, FItemBaseData Item)
{
	InsertItemToIndex(Index, Item);
	InventoryChanged();
}

void UInventoryComponent::Server_SwapItem_Implementation(int32 IndexA, int32 IndexB)
{
	SwapItems(IndexA, IndexB);
	InventoryChanged();
}

void UInventoryComponent::Server_AddItemAmountAtSlot_Implementation(int32 Index, int32 AddedAmount)
{
	AddItemAmountAtSlot(Index, AddedAmount);
	InventoryChanged();
}

void UInventoryComponent::Server_RemoveItemAmountAtSlot_Implementation(int32 Index, int32 AddedAmount)
{
	RemoveItemAmountAtSlot(Index, AddedAmount);
	InventoryChanged();
}

void UInventoryComponent::Server_SwapItemBetweenInventory_Implementation(UInventoryComponent* TargetInventory, int32 TargetIndex, UInventoryComponent* SourceInventory, int32 SourceIndex)
{
	SwapItemBetweenInventory(TargetInventory, TargetIndex, SourceInventory, SourceIndex);
	InventoryChanged();
	TargetInventory->InventoryChanged();
}

void UInventoryComponent::Server_DropItemBetweenInventory_Implementation(UInventoryComponent* TargetInventory,
	int32 TargetIndex, UInventoryComponent* SourceInventory, int32 SourceIndex, FItemBaseData Item)
{
	DropItemBetweenInventory(TargetInventory, TargetIndex, SourceInventory, SourceIndex, Item);
	InventoryChanged();
	TargetInventory->InventoryChanged();
}

void UInventoryComponent::Server_PickUp_Implementation(APickup* Item)
{
	PickupItem(Item);
	InventoryChanged();
}

void UInventoryComponent::OnRep_InventoryContents()
{
	//UE_LOG(LogTemp, Warning, TEXT("INVENTORY REPLICATED"));
	InventoryChanged();
}

void UInventoryComponent::OnRep_CurrentWeight()
{
	InventoryChanged();
}

void UInventoryComponent::Client_AddResult_Implementation(FItemAddResult Result)
{
	AMainPlayer* Player = Cast<AMainPlayer>(GetOwner());
	
	if (Player)
	{
		Player->HUD->AddItemMessage(Result);
		if (Result.OperationResult!=EItemAddedResult::NoItemAdded)
		{
			if (PickUpSound)
			{
				Player->Client_PlaySound2D(PickUpSound);
			}
		}
	}
}

