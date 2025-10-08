// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryComponent.h"

#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "Item/ItemBase.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

UItemBase* UInventoryComponent::FindMatchingItem(UItemBase* Item) const
{
	if (Item)
	{
		//인벤토리에 있는 아이템과 같은 아이템이면 그대로 반환
		if(InventoryContents.Contains(Item)) return Item;
	}
	//아니면 Null 반환
	return nullptr;
}

//아이템 찾기
UItemBase* UInventoryComponent::FindNextItemByID(UItemBase* Item) const
{
	if (Item)
	{
		if (const TArray<TObjectPtr<UItemBase>>::ElementType* Result = InventoryContents.FindByKey(Item))
		{
			return *Result;
		}
		return nullptr;
	}
	return nullptr;
}

//부분 스택 찾기(풀스택이 아닌 아이템 스택)
UItemBase* UInventoryComponent::FindNextPartialStack(UItemBase* Item) const
{
	if (const TArray<TObjectPtr<UItemBase>>::ElementType* Result =
		InventoryContents.FindByPredicate([&Item](const UItemBase* InventoryItem)
		{
			return InventoryItem->ID == Item->ID && !InventoryItem->IsFullStack();
		})
	)
	{
		return *Result;
	}
	return nullptr;
}

void UInventoryComponent::RemoveSingleInstanceOfItem(UItemBase* Item)
{
	InventoryContents.RemoveSingle(Item);
	OnInventoryUpdated.Broadcast();
}

int32 UInventoryComponent::RemoveAmountOfItem(UItemBase* Item, int32 DesiredRemovedAmount)
{
	//삭제하고 싶은 개수와 실제 아이템 개수 중 작은 값
	const int32 ActualAmountToRemove = FMath::Min(DesiredRemovedAmount, Item->Amount);
	//아이템 개수에서 실제 삭제 개수 빼기
	Item->SetAmount(Item->Amount - ActualAmountToRemove);
	//무게에서 삭제된 만큼 빼기
	CurrentWeight -= ActualAmountToRemove * Item->GetItemSingleWeight();
	//그 사실을 널리 알리기
	OnInventoryUpdated.Broadcast();
	//실제 삭제된 개수 반환
	return ActualAmountToRemove;
}

//아이템 스택 쪼개기 함수(마크 우클릭 마냥)
void UInventoryComponent::SplitExistingStack(UItemBase* Item, const int32 AmountToSplit)
{
	//인벤토리 개수를 초과하지 않으면 (1칸 빈 공간이 있으면)
	if (InventoryContents.Num() + 1 <= SlotsCapacity)
	{
		//쪼갤 만큼 삭제하고
		RemoveAmountOfItem(Item, AmountToSplit);
		//쪼갤 만큼 다시 추가
		AddNewItem(Item, AmountToSplit);
	}
}

//아이템 추가 태스크 함수
FItemAddResult UInventoryComponent::HandleAddItem(UItemBase* AddedItem)
{
	if (GetOwner())
	{
		//추가할 아이템 개수
		const int32 RequestedAmount = AddedItem->Amount;

		//여러개 못 드는 아이템일 때 (IsStackable = false 아이템)
		if (!AddedItem->NumericData.IsStackable)
		{
			//단일 아이템 추가 태스크 함수 실행
			return HandleNoneStackableItem(AddedItem);
		}

		//스택 가능 아이템일 때
		//쌓을 개수
		const int32 StackableAmount = HandleStackableItem(AddedItem, RequestedAmount);

		//쌓을 개수가 추가할 개수랑 같으면
		if (StackableAmount == RequestedAmount)
		{
			//몽땅 추가
			return FItemAddResult::AddedAll(RequestedAmount, FText::Format(FText::FromString("Success 아이템 추가 성공 [ {0} : {1} 개 ]"), AddedItem->TextData.Name, RequestedAmount));
		}
		//쌓을 개수가 추가할 개수보다 작거나, 쌓을 개수가 0 초과면
		if (StackableAmount < RequestedAmount && StackableAmount > 0)
		{
			//부분 추가
			return FItemAddResult::AddedPartial(StackableAmount, FText::Format(FText::FromString("Partial 아이템 일부만 추가 [ {0} : {1} 개 ]"), AddedItem->TextData.Name, StackableAmount));
		}
		//쌓을 개수가 0보다 작거나 같다면
		/*if (StackableAmount <= 0)
		{
			//추가 안해부러
			return FItemAddResult::AddedNone(FText::Format(FText::FromString("Failed 아이템 슬롯 개수 초과 [ {0} : {1} 개 ]"), AddedItem->TextData.Name, RequestedAmount));
		}*/
	}
	
	check(false);
	return FItemAddResult::AddedNone(FText::FromString(TEXT("Can't Find Owner")));
}

//단일 아이템 추가 태스크 함수
FItemAddResult UInventoryComponent::HandleNoneStackableItem(UItemBase* AddedItem)
{
	//추가할 아이템 무게 췤. 음수인 지 음수면 아무것도 안 함
	if (FMath::IsNearlyZero(AddedItem->GetItemSingleWeight()) || AddedItem->GetItemSingleWeight() < 0)
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("[WEIGHT ERROR]아이템 무게 오류 [ {0} : {1} ]"),AddedItem->TextData.Name, AddedItem->GetItemSingleWeight()));
	}
	//무게 용량 초과면 아무것도 안 함
	if (CurrentWeight + AddedItem->GetItemSingleWeight() > GetWeightCapacity())
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("[WEIGHT OVERFLOW]인벤토리 용량 초과 [ {0} : {1} ]"),AddedItem->TextData.Name, AddedItem->GetItemSingleWeight()));
	}
	//아이템 슬롯 초과 했능가? 초과면 아무것도 안 함
	if (InventoryContents.Num() + 1 > SlotsCapacity )
	{
		return FItemAddResult::AddedNone(FText::Format(FText::FromString("[SLOT OVERFLOW]인벤토리 슬롯 개수 초과 [ {0} : {1} 개 ]"),AddedItem->TextData.Name, 1));
	}

	//이상할 것이 없으면 추가
	AddNewItem(AddedItem, 1);
	return FItemAddResult::AddedAll(1, FText::Format(FText::FromString("[ADDING SUCCESS]아이템 추가 성공 [ {0} : {1} 개 ]"), AddedItem->TextData.Name, 1));
}

//스택 가능 아이템 추가 태스크 함수
int32 UInventoryComponent::HandleStackableItem(UItemBase* AddedItem, int32 RequestedAmount)
{
	//아이템 개수가 0이거나 음수면 0 반환
	if (RequestedAmount <= 0 || FMath::IsNearlyZero(AddedItem->GetItemStackWeight()))
	{
		return 0;
	}

	//인벤토리에 넣을 양
	int32 AmountToDistribute = RequestedAmount;
	//인벤토리에 있는 풀스택이 아닌 같은 아이템 스택
	UItemBase* ExistingItemStack = FindNextPartialStack(AddedItem);
	
	//인벤토리에 풀스택이 아닌 같은 아이템이 없을 때까지 반복
	while (ExistingItemStack)
	{
		UE_LOG(LogTemp, Warning, TEXT("Enter While"));
		//풀스택까지 남은 개수
		const int32 AmountToMakeFullStack = CalculateAmountForFullStackAmount(ExistingItemStack, AmountToDistribute);
		//무게 초과하지 않는 개수
		const int32 WeightLimitAddAmount = CalculateWeightAddAmount(ExistingItemStack, AmountToMakeFullStack);

		//무게 초과하지 않는 개수가 0 초과면
		if (WeightLimitAddAmount > 0)
		{
			//인벤토리에 있던 아이템 개수 추가
			ExistingItemStack->SetAmount(ExistingItemStack->Amount + WeightLimitAddAmount);
			//개수 추가한 만큼 인벤토리 무게 추가
			CurrentWeight += ExistingItemStack->GetItemSingleWeight() * WeightLimitAddAmount;

			//넣을 양에서 넣은 양 빼기
			AmountToDistribute -= WeightLimitAddAmount;
			//넣을 아이템 개수 수정
			AddedItem->SetAmount(AmountToDistribute);

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
	if (InventoryContents.Num() + 1 <= SlotsCapacity)
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
				AddedItem->SetAmount(AmountToDistribute);

				//아이템 복사본을 무게 초과하지 않는 개수만큼 추가
				AddNewItem(AddedItem->CreateItemCopy(), WeightLimitAddAmount);
				//요청한 개수에서 넣으려는 개수 빼고 반환
				UE_LOG(LogTemp, Warning, TEXT("WeightLimitAmount over 0"));
				return RequestedAmount - AmountToDistribute;
			}
			//아니면 넣으려는 수 만큼 아이템 추가
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
int32 UInventoryComponent::CalculateWeightAddAmount(UItemBase* Item, int32 Amount)
{
	//남은 용량에서 더 추가할 수 있는 개수
	const int32 WeightMaxAddAmount = FMath::FloorToInt((GetWeightCapacity() - CurrentWeight) / Item->GetItemSingleWeight());

	//추가할 아이템이 가능한 개수보다 적으면 추가할 개수 반환
	if (WeightMaxAddAmount >= Amount)
	{
		return Amount;
	}
	//아니면 가능한 개수 반환
	return WeightMaxAddAmount;
}

//스택 아이템 추가 개수 계산 함수
int32 UInventoryComponent::CalculateAmountForFullStackAmount(UItemBase* StackableItem, int32 Amount)
{
	//풀 스택 만들기 위해 남은 개수
	const int32 AmountToMakeFullStack = StackableItem->NumericData.MaxAmount - StackableItem->Amount;

	//둘 중 더 작은 값 반환
	return FMath::Min(Amount, AmountToMakeFullStack);
}

//실직적 아이템 추가 함수
void UInventoryComponent::AddNewItem(UItemBase* Item, const int32 Amount)
{
	UItemBase* NewItem;

	//이미 복사된 아이템이거나 월드에 떨궈진 거면
	if (Item->IsCopy || Item->IsPickup)
	{
		NewItem = Item;
		NewItem->ResetItemFlags();
	} else
	{
		//스택 쪼개기나 드래그해서 옮길 시
		NewItem = Item->CreateItemCopy();
	}
	
	NewItem->OwningInventory = this;
	NewItem->SetAmount(Amount);

	//인벤토리에 추가
	InventoryContents.Add(NewItem);
	//무게 추가
	CurrentWeight += NewItem->GetItemStackWeight();
	//그 사실을 널리 알리기
	OnInventoryUpdated.Broadcast();
}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

