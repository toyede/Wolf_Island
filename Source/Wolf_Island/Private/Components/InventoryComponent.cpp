// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryComponent.h"

#if WITH_EDITOR
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#endif

#include "AdvancedFriendsGameInstance.h"
#include "Item/ItemBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	//SetIsReplicated(true);
	
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
		if (Slot.Item)
		{
			if (Slot.Item->ID == ItemID)
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
	FText ItemName = FText();
	for (FItemSlot Slot : InventoryContents)
	{
		if (Slot.Item)
		{
			if (Slot.Item->ID == ItemID)
			{
				ItemName = Slot.Item->TextData.Name;
				count += Slot.Item->Amount;
			}
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("[ %s ] : %d in Inventory."), *ItemName.ToString() ,count);
	return count;
}

void UInventoryComponent::SetItemAtIndex(FItemBaseData* Item, int32 Index)
{
	FItemBaseData RemovedItem = GetItemAtIndex(Index);
	
	if (RemovedItem.IsValid())
	{
		CurrentWeight -= GetItemSingleWeight(RemovedItem) * RemovedItem.Amount;
	}
	if (Item)
	{
		CurrentWeight += GetItemSingleWeight(*Item) * Item->Amount;
		InventoryContents[Index].ItemData = *Item;
		
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("NO INPUT ITEM"));
		InventoryContents[Index].Clear();
	}
	OnInventoryUpdated.Broadcast();
}

FItemBaseData* UInventoryComponent::FindMatchingItem(FItemBaseData& Item) const
{
	if (Item.IsValid())
	{
		//인벤토리에 있는 아이템과 같은 아이템이면 그대로 반환
		//if(InventoryContents.Contains(Item)) return Item;
		for (FItemSlot Slot : InventoryContents)
		{
			if (Slot.Item)
			{
				if (Slot.Item->ID == Item.ItemID)
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


int32 UInventoryComponent::RemoveAmountOfItem(FItemBaseData& Slot, int32 DesiredRemovedAmount)
{
	UE_LOG(LogTemp, Warning, TEXT("Item Amount : %d | Desired Amount : %d"), Slot.Amount, DesiredRemovedAmount);
	//삭제하고 싶은 개수와 실제 아이템 개수 중 작은 값
	const int32 ActualAmountToRemove = FMath::Min(DesiredRemovedAmount, Slot.Amount);
	//아이템 개수에서 실제 삭제 개수 빼기
	Slot.SetAmount(Slot.Amount - ActualAmountToRemove);
	//무게에서 삭제된 만큼 빼기
	CurrentWeight -= ActualAmountToRemove * GetItemSingleWeight(Slot.ItemID);
	//그 사실을 널리 알리기
	OnInventoryUpdated.Broadcast();
	//실제 삭제된 개수 반환
	return ActualAmountToRemove;
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
FItemAddResult UInventoryComponent::HandleAddItem(FItemBaseData& AddedItem)
{
	//클라이언트 실행
	if (!GetOwner()->HasAuthority())
	{
		Server_HandleAddItem(AddedItem);
		
		return FItemAddResult::AddedNone(
			FText::FromString(TEXT("Request Sent")),
			EItemFailReason::NoReason);
	}
	
	//서버 실행
	return Internal_HandleAddItem(AddedItem);
}

//단일 아이템 추가 태스크 함수
FItemAddResult UInventoryComponent::HandleNoneStackableItem(FItemBaseData& AddedItem)
{
	//추가할 아이템 무게 췤. 음수인 지 음수면 아무것도 안 함
	if (FMath::IsNearlyZero(GetItemSingleWeight(AddedItem)) || GetItemSingleWeight(AddedItem) < 0)
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("[WEIGHT ERROR] [ {0} : x{1} ]"),GetItemData(AddedItem)->TextData.Name, GetItemSingleWeight(AddedItem)), EItemFailReason::SystemError);
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
			//개수 추가한 만큼 인벤토리 무게 추가
			CurrentWeight += GetItemSingleWeight(*ExistingItemStack) * WeightLimitAddAmount;

			//넣을 양에서 넣은 양 빼기
			AmountToDistribute -= WeightLimitAddAmount;
			//넣을 아이템 개수 수정
			AddedItem.SetAmount(AmountToDistribute);

			//무게 용량 초과면
			if (CurrentWeight >= WeightCapacity)
			{
				OnInventoryUpdated.Broadcast();
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
				OnInventoryUpdated.Broadcast();
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
			OnInventoryUpdated.Broadcast();
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

	OnInventoryUpdated.Broadcast();
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
	FItemBaseData NewItem = FItemBaseData();

	/*//이미 복사된 아이템이거나 월드에 떨궈진 거면
	if (Item->IsCopy || Item->IsPickup)
	{
		NewItem = Item;
		NewItem->ResetItemFlags();
	} else
	{
		//스택 쪼개기나 드래그해서 옮길 시
		NewItem = Item->CreateItemCopy();
	}
	
	NewItem->OwningInventory = this;*/
	NewItem.SetAmount(Amount);

	//인벤토리에 추가
	//InventoryContents.Add(NewItem);
	FindEmptySlot()->ItemData = NewItem;
	
	//무게 추가
	CurrentWeight += GetItemStackWeight(NewItem);
	//그 사실을 널리 알리기
	OnInventoryUpdated.Broadcast();
}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	//나중에 저장 데이터에서 인벤토리 가져오는 코드 구현 예정
	

	InventoryContents.Init(FItemSlot(), SlotsCapacity);
	// ...
	
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
		if (!Slot.Item)
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
		if (Slot.Item)
		{
			if (Slot.Item->ID == ItemID)
			{
				int32 RemoveAmount = FMath::Min(DesiredRemoveAmount, Slot.Item->Amount);
				Slot.Item->Amount -= RemoveAmount;
				DesiredRemoveAmount -= RemoveAmount;

				UE_LOG(LogTemp, Log, TEXT("Removed %d of %s, Remaining remove amount: %d"), RemoveAmount, *Slot.Item->TextData.Name.ToString(), DesiredRemoveAmount);

				if (Slot.Item->Amount <= 0) Slot.Item = nullptr;
				if (DesiredRemoveAmount <= 0) break;
			}
		}
	}
	
	OnInventoryUpdated.Broadcast();
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

	//아이템 데이터 반환
	return NewItem;
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
				
			}
			//있으면 추가
			else
			{
				AddNewItem(ResultItem, ResultItem.Amount);
			}
			
			return true;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Recipe Check Failed."));
	
	return false;
}

bool UInventoryComponent::RepairShip(FRepairRecipeData Recipes)
{
	UE_LOG(LogTemp, Warning, TEXT("Make Item Execute."));
	//레시피 다시한번 체크
	if (CheckCanMakeRepair(Recipes))
	{
		//레시피 개수별로 아이템 삭제
		RemoveItemsByID(Recipes.Ingredient1ID, Recipes.Ingredient1Amount);
		RemoveItemsByID(Recipes.Ingredient2ID, Recipes.Ingredient2Amount);
		RemoveItemsByID(Recipes.Ingredient3ID, Recipes.Ingredient3Amount);
		RemoveItemsByID(Recipes.Ingredient4ID, Recipes.Ingredient4Amount);
	}
	UE_LOG(LogTemp, Warning, TEXT("Recipe Check Failed."));
	
	return false;
}

void UInventoryComponent::InsertItemToIndex(int32 Index, FItemBaseData Item)
{
	if (InventoryContents.IsValidIndex(Index))
	{
		InventoryContents[Index].ItemData = Item;
		
		OnInventoryUpdated.Broadcast();
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
	
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::SwapItemsBetweenInventory(
	UInventoryComponent* OriginInventoryComponent, int32 OriginIndex,
	UInventoryComponent* TargetInventoryComponent, int32 TargetIndex)
{
	FItemSlot& OriginSlot = OriginInventoryComponent->InventoryContents[OriginIndex];
	FItemSlot& TargetSlot = TargetInventoryComponent->InventoryContents[TargetIndex];
	
	//서로 다른 아이템이거나 하나라도 빈 슬롯이면 그냥자리 교환
	if (OriginSlot.IsEmpty() || TargetSlot.IsEmpty() || OriginSlot.ItemData.ItemID != TargetSlot.ItemData.ItemID)
	{
		//아이템 소유 인벤토리 변경 및 무게 증감
		if (OriginSlot.IsNotEmpty())
		{
			OriginInventoryComponent->CurrentWeight -= GetItemSingleWeight(OriginSlot.ItemData) * OriginSlot.ItemData.Amount;
			TargetInventoryComponent->CurrentWeight += GetItemSingleWeight(OriginSlot.ItemData) * OriginSlot.ItemData.Amount;
		}
		if (TargetSlot.IsNotEmpty())
		{
			OriginInventoryComponent->CurrentWeight += GetItemSingleWeight(TargetSlot.ItemData) * TargetSlot.ItemData.Amount;
			TargetInventoryComponent->CurrentWeight -= GetItemSingleWeight(TargetSlot.ItemData) * TargetSlot.ItemData.Amount;
		}
		
		Swap(OriginSlot, TargetSlot);
	}
	//같은 아이템이면 스택 확인
	else
	{
		// 같은 아이템이면 스택 합치기
		//분배할 총 개수
		int32 TotalAmount = OriginSlot.ItemData.Amount + TargetSlot.ItemData.Amount;
		//최대 스택 개수
		int32 MaxStack = GetItemMaxAmount(TargetSlot.ItemData);

		//일단 무게 빼기
		OriginInventoryComponent->CurrentWeight -= GetItemSingleWeight(OriginSlot.ItemData) * OriginSlot.ItemData.Amount;
		TargetInventoryComponent->CurrentWeight -= GetItemSingleWeight(TargetSlot.ItemData) * TargetSlot.ItemData.Amount;
		
		//드래그 가져온 슬롯에 총 개수와 최대 스택 개수 중 작은 것 할당
		OriginSlot.ItemData.Amount = FMath::Min(TotalAmount, MaxStack);
		//드롭 받는 슬롯에 총 개수 - 가져온 슬롯 개수 할당
		TargetSlot.ItemData.Amount = TotalAmount - OriginSlot.ItemData.Amount;

		//분배한 만큼 무게 추가
		OriginInventoryComponent->CurrentWeight += GetItemSingleWeight(OriginSlot.ItemData) * OriginSlot.ItemData.Amount;
		TargetInventoryComponent->CurrentWeight += GetItemSingleWeight(TargetSlot.ItemData) * TargetSlot.ItemData.Amount;

		if (TargetSlot.ItemData.Amount <= 0){
			TargetSlot.Clear();
		}
	}
	
	OriginInventoryComponent->OnInventoryUpdated.Broadcast();
	TargetInventoryComponent->OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::DropItemBetweenInventory(
	UInventoryComponent* OriginInventoryComponent, int32 OriginIndex,
	UInventoryComponent* TargetInventoryComponent, int32 TargetIndex,
	FItemBaseData DraggedItem)
{
	//드래그 가져온 슬롯
	FItemSlot& OriginSlot = OriginInventoryComponent->InventoryContents[OriginIndex];
	//드롭 받는 슬롯
	FItemSlot& TargetSlot = TargetInventoryComponent->InventoryContents[TargetIndex];
	
	//빈 슬롯이면 삽입
	if (TargetSlot.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("DIBI: EMPTY SLOT INSERT"));
		TargetInventoryComponent->InsertItemToIndex(TargetIndex, DraggedItem);

		//무게 업데이트
		OriginInventoryComponent->CurrentWeight -= GetItemSingleWeight(DraggedItem.ItemID) * DraggedItem.Amount;
		TargetInventoryComponent->CurrentWeight += GetItemSingleWeight(DraggedItem.ItemID) * DraggedItem.Amount;
		
		OriginInventoryComponent->OnInventoryUpdated.Broadcast();
		TargetInventoryComponent->OnInventoryUpdated.Broadcast();
		
		return;
	}
	//다른 아이템이면 원상복구
	if (TargetSlot.ItemData.ItemID != DraggedItem.ItemID)
	{
		UE_LOG(LogTemp, Warning, TEXT("DIBI: BACK TO NORMAL"));
		/*UItemBase* OriginItem = OriginSlot.Item;
		OriginItem->Amount += DraggedItem->Amount;

		OriginInventoryComponent->OnInventoryUpdated.Broadcast();
		TargetInventoryComponent->OnInventoryUpdated.Broadcast();*/

		return;
	}
	//같은 아이템이면 연산 후 정리
	//분배할 총 개수
	int32 TotalAmount = OriginSlot.ItemData.Amount + DraggedItem.Amount;
	//최대 스택 개수
	int32 MaxStack = GetItemMaxAmount(TargetSlot.ItemData.ItemID);

	//일단 무게 빼기
	OriginInventoryComponent->CurrentWeight -= GetItemSingleWeight(OriginSlot.ItemData) * OriginSlot.ItemData.Amount;
	TargetInventoryComponent->CurrentWeight -= GetItemSingleWeight(TargetSlot.ItemData) * TargetSlot.ItemData.Amount;

	//드롭 받는 슬롯에 총 개수와 최대 스택 개수 중 작은 것 할당
	TargetSlot.ItemData.Amount = FMath::Min(TotalAmount, MaxStack);
	//드래그 가져온 슬롯에 총 개수 - 드롭 받는 슬롯 개수 할당
	OriginSlot.ItemData.Amount = TotalAmount - TargetSlot.ItemData.Amount;

	//무게 업데이트
	OriginInventoryComponent->CurrentWeight += GetItemSingleWeight(OriginSlot.ItemData) * OriginSlot.ItemData.Amount;
	TargetInventoryComponent->CurrentWeight += GetItemSingleWeight(TargetSlot.ItemData) * TargetSlot.ItemData.Amount;

	if (OriginSlot.ItemData.Amount <= 0)
	{
		OriginSlot.Clear();
	}

	OriginInventoryComponent->OnInventoryUpdated.Broadcast();
	TargetInventoryComponent->OnInventoryUpdated.Broadcast();
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

void UInventoryComponent::Server_HandleAddItem_Implementation(FItemBaseData AddedItem)
{
	Internal_HandleAddItem(AddedItem);
}

FItemAddResult UInventoryComponent::Internal_HandleAddItem(FItemBaseData& AddedItem)
{
	if (GetOwner())
	{
		//추가할 아이템 개수
		const int32 RequestedAmount = AddedItem.Amount;

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

void UInventoryComponent::OnRep_InventoryContents()
{
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::OnRep_CurrentWeight()
{
	OnInventoryUpdated.Broadcast();
}

