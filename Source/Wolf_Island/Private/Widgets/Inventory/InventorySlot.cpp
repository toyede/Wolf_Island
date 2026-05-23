// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/InventorySlot.h"

#include "Character/MainPlayer.h"
#include "Components/InventoryComponent.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
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
	
	if (OwnerInventoryRef)
	{
		OwnerInventoryRef->OnInventoryUpdated.AddUObject(this, &UInventorySlot::RefreshSlot);
	}
	
	//SetEmptySlot();
}

void UInventorySlot::SetSelectedSlot()
{
	ItemBorder->SetBrush(SelectedSlotBrush);
}

void UInventorySlot::SetUnSelectedSlot()
{
	ItemBorder->SetBrush(UnSelectedSlotBrush);
}

void UInventorySlot::SetEmptySlot()
{
	ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
	ItemAmount->SetVisibility(ESlateVisibility::Collapsed);
	if (DurabilityBar)
	{
		DurabilityBar->SetVisibility(ESlateVisibility::Collapsed);
		DurabilityBar->SetPercent(0.0f);
	}
}

void UInventorySlot::RefreshSlot()
{
	// OwnerInventoryRef가 null이면 (HotBar 슬롯이 아직 초기화되지 않았거나 맵 전환 중 정리된 경우) 안전하게 종료
	if (!IsValid(OwnerInventoryRef))
	{
		return;
	}
	
	//UE_LOG(LogTemp, Warning, TEXT("%s's [ %d ] SLOT REFRESHING"), *OwnerInventoryRef->GetOwner()->GetName() ,Index);
	ItemRef = OwnerInventoryRef->GetItemAtIndex(Index);
	
	ItemData = OwnerInventoryRef->GetItemData(ItemRef);
	
	if (ItemData)
	{
		ItemIcon->SetVisibility(ESlateVisibility::Visible);
		ItemIcon->SetBrushFromTexture(ItemData->AssetData.Icon);

		if (ItemData->NumericData.IsStackable)
		{
			ItemAmount->SetVisibility(ESlateVisibility::Visible);
			ItemAmount->SetText(FText::AsNumber(ItemRef.Amount));
			
			if (ItemRef.Amount == 1)
			{
				//ItemAmount->SetVisibility(ESlateVisibility::Collapsed);
			}
		} else
		{
			ItemAmount->SetVisibility(ESlateVisibility::Collapsed);
		}
	} else
	{
		//UE_LOG(LogTemp, Warning, TEXT(">> NO ITEM DATA IN SLOT <<"));
		SetEmptySlot();
	}

	if (DurabilityBar)
	{
		bool bShowDurability = false;
		float Percent = 0.0f;

		if (ItemData)
		{
			const bool bUseBar = (ItemData->Type == EItemType::EQUIPMENT) || ItemData->ID == FName("FO019");

			if (bUseBar && ItemRef.MaxDurability > 0.0f)
			{
				Percent = FMath::Clamp(ItemRef.CurrentDurability / ItemRef.MaxDurability, 0.0f, 1.0f);

				bShowDurability = (ItemRef.CurrentDurability < ItemRef.MaxDurability);

				FLinearColor FillColor = FLinearColor(0.1f, 0.85f, 0.2f, 1.0f);
				if (Percent <= 0.10f || ItemRef.CurrentDurability == 1.0f)
				{
					FillColor = FLinearColor(0.9f, 0.1f, 0.1f, 1.0f);
				}
				else if (Percent <= 0.30f)
				{
					FillColor = FLinearColor(0.95f, 0.8f, 0.1f, 1.0f);
				}

				DurabilityBar->SetFillColorAndOpacity(FillColor);
			}
		}

		DurabilityBar->SetVisibility(bShowDurability ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		DurabilityBar->SetPercent(Percent);
	}
	
	if (CanDragDrop && ToolTipClass && ItemRef.IsValid())
	{
		UInventoryToolTip* ToolTip = CreateWidget<UInventoryToolTip>(this, ToolTipClass);
		ToolTip->InventorySlotBeingHovered = this;
			
		if (ItemData)
		{
			ToolTip->ItemData = ItemData;
		}
			
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

FReply UInventorySlot::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	UE_LOG(LogTemp, Warning, TEXT("[INVEN SLOT] DOUBLE CLICKED"))
	
	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
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
		if (DragItemVisualClass && ItemRef.IsValid() && ItemData)
		{			
			const TObjectPtr<UDragItemVisual> DragVisual = CreateWidget<UDragItemVisual>(this, DragItemVisualClass);
			DragVisual->ItemIcon->SetBrushFromTexture(ItemData->AssetData.Icon);
			DragVisual->ItemBorder->SetBrush(UnSelectedSlotBrush);
			DragVisual->ItemAmount->SetText(FText::AsNumber(ItemRef.Amount));
			if (ItemRef.Amount == 1)
				DragVisual->ItemAmount->SetVisibility(ESlateVisibility::Collapsed);

			UItemDragDropOperation* DragItemOperation = NewObject<UItemDragDropOperation>();
			DragItemOperation->SourceInventory = OwnerInventoryRef;
			DragItemOperation->SourceIndex = Index;
			DragItemOperation->SourceItemData = ItemRef;

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
		if (DragItemVisualClass && ItemRef.IsValid())
		{
			//반갈 개수
			int32 MovedAmount = ItemRef.Amount/2;
			//1개면 우클릭 불가능
			if (MovedAmount == 0 ) return;
			
			//기존 슬롯에 남은 아이템 개수
			int32 RemaindAmount = ItemRef.Amount - MovedAmount;

			//기존 슬롯 아이템 개수 설정 - 드래그 떼어 간 만큼 감소(무게 제외 수량만 감소)
			OwnerInventoryRef->Request_RemoveOnlyItemAmountAtSlot(Index, MovedAmount);

			//드래그 아이템 위젯 생성 (비주얼만 만드는 거임)
			const TObjectPtr<UDragItemVisual> DragVisual = CreateWidget<UDragItemVisual>(this, DragItemVisualClass);
			DragVisual->ItemIcon->SetBrushFromTexture(ItemData->AssetData.Icon);
			DragVisual->ItemBorder->SetBrush(UnSelectedSlotBrush);
			DragVisual->ItemAmount->SetText(FText::AsNumber(MovedAmount));
			if (ItemRef.Amount == 1)
				DragVisual->ItemAmount->SetVisibility(ESlateVisibility::Collapsed);

			//드래그 아이템 데이터 생성
			UItemDragDropOperation* DragItemOperation = NewObject<UItemDragDropOperation>();
			DragItemOperation->SourceInventory = OwnerInventoryRef;
			DragItemOperation->SourceItemData = ItemRef;
			DragItemOperation->SourceItemData.Amount = MovedAmount;
			DragItemOperation->SourceIndex = Index;
			
			DragItemOperation->DefaultDragVisual = DragVisual;
			DragItemOperation->Pivot = EDragPivot::MouseDown;

			RefreshSlot();
			
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
				
				UE_LOG(LogTemp, Warning, TEXT("Drop %d Item on %d"), ItemDragDrop->SourceIndex, Index);
				//다른 칸이면 스왑
				//TODO: 서버 호출 함수로 변경
				OriginInventoryRef->Request_SwapItem(ItemDragDrop->SourceIndex, Index);
				return true;
			}
			//우클릭 떨구기면 연산 후 삽입
			else if (InDragDropEvent.GetEffectingButton() == EKeys::RightMouseButton)
			{
				//같은 아이템이 아니면
				if (!OriginInventoryRef->CheckSameItemAtIndex(Index, ItemDragDrop->SourceItemData.ItemID))
				{
					UE_LOG(LogTemp, Warning, TEXT("RIGHT SLOT DROP DETECTED"));
					//빈 슬롯이면 그대로 삽입
					//TODO: 서버 호출 함수로 변경
					if (OriginInventoryRef->CheckEmptySlotAtIndex(Index))
					{
						//슬롯에 아이템 넣기
						OriginInventoryRef->Request_SetItemAtSlot(Index, ItemDragDrop->SourceItemData);
						//TODO: 서버 호출 함수로 변경
						ItemDragDrop->SourceInventory->OnInventoryUpdated.Broadcast();
						
						return true;
					}
					
					//아니면 제자리로
					//원래 인덱스 아이템 개수 원상 복구 후 끝
					//TODO: 서버 호출 함수로 변경
					OriginInventoryRef->Request_AddItemAmountAtSlot(ItemDragDrop->SourceIndex, ItemDragDrop->SourceItemData.Amount);
				
					return true;
				}
				//같은 아이템이면 연산 후 삽입
				else
				{
					//옮길 곳에 최대로 더하고 남은 건 원래 자리로
					//옮길(드롭 받는) 곳 : Index | 옮기는(드래그 한) 것 : SourceIndex
					//총 분배 개수 - 드롭 받는 곳의 아이템 개수와 드래그 한 아이템 개수의 합
					int32 TotalAmount = ItemRef.Amount + ItemDragDrop->SourceItemData.Amount;
					//최대 스택 개수
					int32 MaxStack = ItemData->NumericData.MaxAmount;

					//최대 스택 개수랑 총 분배할 개수 중 작은 것을 드롭 받는 슬롯에 설정
					OriginInventoryRef->Request_SetItemAmountAtSlot(Index, FMath::Min(TotalAmount, MaxStack));
					
					//남은 아이템 개수는 총 분배 개수 - 드롭 받는 슬롯의 아이템 개수
					int32 Remained = TotalAmount - FMath::Min(TotalAmount, MaxStack);
					//TODO: 서버 호출 함수로 변경
					OriginInventoryRef->Request_AddItemAmountAtSlot(ItemDragDrop->SourceIndex, Remained);
					
					return true;
				}
			}
		}
		//다른 인벤토리 간 이동이면, (ex. 플레이어 <-> 상자)
		else
		{
			UInventoryComponent* RCPInventory = OwnerActor->GetComponentByClass<UInventoryComponent>();
			
			if (!RCPInventory) return false;
			
			//좌클릭 떨구기면 스왑
			if (InDragDropEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			{
				UE_LOG(LogTemp, Warning, TEXT("LEFT DROP BETWEEN OTHERS"))
				//TODO: 서버 호출 함수로 변경
				//OriginInventoryRef는 외부에서 온 슬롯의 인벤토리
				//드롭받는 현재 인벤토리의 슬롯과 외부에서 온 슬롯과 교환하는 코드이다.
				RCPInventory->Request_SwapItemBetweenInventory(
					OwnerInventoryRef,Index, 
					OriginInventoryRef, ItemDragDrop->SourceIndex);
				
				return true;
			}
			//우클릭 떨구기면 연산 후 삽입
			else if (InDragDropEvent.GetEffectingButton() == EKeys::RightMouseButton)
			{
				UE_LOG(LogTemp, Warning, TEXT("RIGHT DROP BETWEEN OTHERS"))
				//TODO: 서버 호출 함수로 변경
				//외부에서 온 슬롯이 드롭받는 슬롯에 떨어졌을 때 연산
				RCPInventory->Request_DropItemBetweenInventory(
					OwnerInventoryRef,Index, 
					OriginInventoryRef, ItemDragDrop->SourceIndex, 
					ItemDragDrop->SourceItemData);

				return true;
			}
		}
	}
	
	return false;
}