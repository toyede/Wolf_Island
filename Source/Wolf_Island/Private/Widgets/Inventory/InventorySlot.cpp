// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/InventorySlot.h"

#include "Components/InventoryComponent.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Item/ItemBase.h"
#include "Widgets/Inventory/DragItemVisual.h"
#include "Widgets/Inventory/InventoryToolTip.h"
#include "Widgets/Inventory/ItemDragDropOperation.h"

void UInventorySlot::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetUnSelectedSlot();
}


void UInventorySlot::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemRef)
	{
		ItemIcon->SetBrushFromTexture(ItemRef->AssetData.Icon);

		if (ItemRef->NumericData.IsStackable)
		{
			ItemAmount->SetVisibility(ESlateVisibility::Visible);
			ItemAmount->SetText(FText::AsNumber(ItemRef->Amount));
		} else
		{
			ItemAmount->SetVisibility(ESlateVisibility::Collapsed);
		}
	} else
	{
		ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
		ItemAmount->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (ToolTipClass && ItemRef && CanDragDrop)
	{
		UInventoryToolTip* ToolTip = CreateWidget<UInventoryToolTip>(this, ToolTipClass);
		ToolTip->InventorySlotBeingHovered = this;
		SetToolTip(ToolTip);
	}
}

FReply UInventorySlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	if (!CanDragDrop) return Reply.Unhandled();
	
	//왼쪽 마우스 클릭이면
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return Reply.Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}
	//오른쪽 마우스 클릭이면
	else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		return Reply.Handled().DetectDrag(TakeWidget(), EKeys::RightMouseButton);
	}

	return Reply.Unhandled();
}

void UInventorySlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
}

void UInventorySlot::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	
	if (!CanDragDrop) return;

	//좌클릭 드래그면 이동
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		if (DragItemVisualClass && ItemRef)
		{
			const TObjectPtr<UDragItemVisual> DragVisual = CreateWidget<UDragItemVisual>(this, DragItemVisualClass);
			DragVisual->ItemIcon->SetBrushFromTexture(ItemRef->AssetData.Icon);
			DragVisual->ItemBorder->SetBrush(UnSelectedSlotBrush);
			DragVisual->ItemAmount->SetText(FText::AsNumber(ItemRef->Amount));

			UItemDragDropOperation* DragItemOperation = NewObject<UItemDragDropOperation>();
			DragItemOperation->SourceItem = ItemRef;
			DragItemOperation->SourceInventory = ItemRef->OwningInventory;
			DragItemOperation->SourceIndex = Index;

			DragItemOperation->DefaultDragVisual = DragVisual;
			DragItemOperation->Pivot = EDragPivot::MouseDown;

			OutOperation = DragItemOperation;
			if (OutOperation)
			{
				UE_LOG(LogTemp, Warning, TEXT("LEFT DRAG ITEM OPERATION OUTTED"));
			}
		}
	}
	//우클릭 드래그면 반으로 나눠보리기
	else if (InMouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
	{
		if (DragItemVisualClass && ItemRef)
		{
			int32 MovedAmount = ItemRef->Amount/2;
			int32 RemaindAmount = ItemRef->Amount - MovedAmount;

			if (MovedAmount == 0 ) return;
			
			ItemRef->Amount = RemaindAmount;			
			
			const TObjectPtr<UDragItemVisual> DragVisual = CreateWidget<UDragItemVisual>(this, DragItemVisualClass);
			DragVisual->ItemIcon->SetBrushFromTexture(ItemRef->AssetData.Icon);
			DragVisual->ItemBorder->SetBrush(UnSelectedSlotBrush);
			DragVisual->ItemAmount->SetText(FText::AsNumber(MovedAmount));

			UItemDragDropOperation* DragItemOperation = NewObject<UItemDragDropOperation>();
			DragItemOperation->SourceItem = ItemRef->CreateItemCopy();
			DragItemOperation->SourceInventory = ItemRef->OwningInventory;
			DragItemOperation->SourceIndex = Index;
			DragItemOperation->SourceItem->Amount = MovedAmount;
			DragItemOperation->SourceItem->OwningInventory = ItemRef->OwningInventory;

			DragItemOperation->DefaultDragVisual = DragVisual;
			DragItemOperation->Pivot = EDragPivot::MouseDown;

			ItemRef->OwningInventory->OnInventoryUpdated.Broadcast();
			
			OutOperation = DragItemOperation;
			if (OutOperation)
			{
				UE_LOG(LogTemp, Warning, TEXT("RIGHT DRAG ITEM OPERATION OUTTED"));
			}
		}
	}
}

bool UInventorySlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	if (!CanDragDrop) return false;
	
	const UItemDragDropOperation* ItemDragDrop = Cast<UItemDragDropOperation>(InOperation);
	UE_LOG(LogTemp, Warning, TEXT("SLOT DROP DETECTED"));
	
	UInventoryComponent* OriginInventoryRef = ItemDragDrop->SourceInventory;
	
	if (OriginInventoryRef)
	{
		//같은 인벤토리 안에서 이동이면,
		if (OwnerInventoryRef == OriginInventoryRef)
		{
			UE_LOG(LogTemp, Warning, TEXT("SLOT DROPPED AT SAME INVENTORY"));
			//좌클릭 떨구기면 스왑
			if (InDragDropEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			{
				UE_LOG(LogTemp, Warning, TEXT("LEFT SLOT DROP DETECTED"));
				//같은 칸이면 무시
				if (ItemDragDrop->SourceIndex == Index) return false;

				//다른 칸이면 스왑
				OriginInventoryRef->SwapItems(ItemDragDrop->SourceIndex, Index);
				return true;
			}
			//우클릭 떨구기면 연산 후 삽입
			else if (InDragDropEvent.GetEffectingButton() == EKeys::RightMouseButton)
			{
				//같은 아이템이 아니면
				if (!OriginInventoryRef->CheckSameItemAtIndex(Index, ItemDragDrop->SourceItem))
				{
					UE_LOG(LogTemp, Warning, TEXT("RIGHT SLOT DROP DETECTED"));
					//빈 슬롯이면 그대로 삽입
					if (OriginInventoryRef->CheckEmptySlotAtIndex(Index))
					{
						OriginInventoryRef->InsertItemToIndex(Index, ItemDragDrop->SourceItem);
						return true;
					}
					//아니면 제자리로
					//원래 인덱스 아이템 개수 원상 복구 후 끝
					OriginInventoryRef->GetItemAtIndex(ItemDragDrop->SourceIndex)->Amount += ItemDragDrop->SourceItem->Amount;
					OriginInventoryRef->OnInventoryUpdated.Broadcast();
				
					return true;
				}
				//같은 아이템이면 연산 후 삽입
				else
				{
					//옮길 곳에 최대로 더하고 남은 건 원래 자리로
					//옮길 곳 : Index | 옮기는 것 : SourceIndex
					//총 분배 개수
					int32 TotalAmount = ItemRef->Amount + ItemDragDrop->SourceItem->Amount;
					//최대 스택 개수
					int32 MaxStack = ItemRef->NumericData.MaxAmount;

					//최대 스택 개수랑 총 분배할 개수 중 작은 것을 기존 슬롯에 분배
					ItemRef->Amount = FMath::Min(TotalAmount, MaxStack);
					ItemDragDrop->SourceItem->Amount = TotalAmount - ItemRef->Amount;
					OriginInventoryRef->GetItemAtIndex(ItemDragDrop->SourceIndex)->Amount += ItemDragDrop->SourceItem->Amount;
					OriginInventoryRef->OnInventoryUpdated.Broadcast();
				}
			}
		}
		//다른 인벤토리 간 이동이면, (ex. 플레이어 <-> 상자)
		else
		{
			//좌클릭 떨구기면 스왑
			if (InDragDropEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			{
				OwnerInventoryRef->SwapItemsBetweenInventory(
					OwnerInventoryRef, Index,
					OriginInventoryRef, ItemDragDrop->SourceIndex);
				
				return true;
			}
			//우클릭 떨구기면 연산 후 삽입
			else if (InDragDropEvent.GetEffectingButton() == EKeys::RightMouseButton)
			{
				OwnerInventoryRef->DropItemBetweenInventory(
					OriginInventoryRef, ItemDragDrop->SourceIndex,
					OwnerInventoryRef, Index,
					ItemDragDrop->SourceItem);
			}
		}
	}
	
	return false;
}

void UInventorySlot::SetSelectedSlot()
{
	ItemBorder->SetBrush(SelectedSlotBrush);
}

void UInventorySlot::SetUnSelectedSlot()
{
	ItemBorder->SetBrush(UnSelectedSlotBrush);
}
