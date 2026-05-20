// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Inventory.h"

#include "Character/MainPlayer.h"
#include "Components/InventoryComponent.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/StatusComponent.h"
#include "Widgets/Record/UnknownRecordPanel.h"
#include "Components/WidgetSwitcher.h"
#include "Widgets/TextButton.h"
#include "Widgets/Craft/BuildingPanel.h"
#include "Widgets/Craft/CraftPanel.h"
#include "Widgets/Inventory/InventoryPanel.h"
#include "Widgets/Inventory/ItemDragDropOperation.h"

void UInventory::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	PanelSwitcher->AddChild(CraftRecipeSection);
	PanelSwitcher->AddChild(FoodRecipeSection);
	PanelSwitcher->AddChild(BuildingRecipeSection);
	PanelSwitcher->AddChild(UnknownRecordSection);
	
	CraftRecipeButton->OnClicked.AddDynamic(this, &UInventory::HandleCraftRecipeClicked);
	FoodRecipeButton->OnClicked.AddDynamic(this, &UInventory::HandleFoodRecipeClicked);
	BuildingRecipeButton->OnClicked.AddDynamic(this, &UInventory::HandleBuildingRecipeClicked);
	UnknownRecordButton->OnClicked.AddDynamic(this, &UInventory::HandleUnknownRecordClicked);
	
	CurrentPanelButton = CraftRecipeButton;
	CurrentPanelButton->SetSelected(true);
}

void UInventory::NativeConstruct()
{
	Super::NativeConstruct();

	PlayerRef = Cast<AMainPlayer>(GetOwningPlayerPawn());
	
	if (PlayerRef && PlayerRef->StatusComponent)
	{
		PlayerRef->StatusComponent->OnHPPercentChanged.AddDynamic(this, &UInventory::UpdateHP);
		PlayerRef->StatusComponent->OnStaminaPercentChanged.AddDynamic(this, &UInventory::UpdateStamina);
		PlayerRef->StatusComponent->OnHungerPercentChanged.AddDynamic(this, &UInventory::UpdateHunger);
		PlayerRef->StatusComponent->OnHydrationPercentChanged.AddDynamic(this, &UInventory::UpdateHydration);
		PlayerRef->StatusComponent->OnInfectionPercentChanged.AddDynamic(this, &UInventory::UpdateInfection);
		
		UpdateHP(PlayerRef->StatusComponent->GetHPPercent());
		UpdateStamina(PlayerRef->StatusComponent->GetStaminaPercent());
		UpdateHunger(PlayerRef->StatusComponent->GetHungerPercent());
		UpdateHydration(PlayerRef->StatusComponent->GetHydrationPercent());
		UpdateInfection(PlayerRef->StatusComponent->GetInfectionPercent());
	}
	
	if (UCraftPanel* CraftPanel = Cast<UCraftPanel>(CraftRecipeSection))
	{
		CraftPanel->SetCraftingMethod(ECraftMethod::INVEN);
	}
}

bool UInventory::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	const UItemDragDropOperation* ItemDragDrop = Cast<UItemDragDropOperation>(InOperation);

	const FItemBaseData ItemData = ItemDragDrop->SourceItemData;
	
	if (PlayerRef && ItemData.IsValid())
	{
		if (InventoryPanel)
		{
			FVector2D LocalMousePos = InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
			
			FGeometry PanelGeometry = InventorySection->GetCachedGeometry();
			FVector2D PanelPos = InGeometry.AbsoluteToLocal(PanelGeometry.GetAbsolutePosition());
			FVector2D PanelSize = PanelGeometry.GetLocalSize();

			// 마우스가 패널 영역 안에 있다면 드랍 무시
			if (LocalMousePos.X >= PanelPos.X && LocalMousePos.X <= PanelPos.X + PanelSize.X &&
				LocalMousePos.Y >= PanelPos.Y && LocalMousePos.Y <= PanelPos.Y + PanelSize.Y)
			{
				//우클릭이면 떨구기면 반갈한 거 원위치
				if (InDragDropEvent.GetEffectingButton() == EKeys::RightMouseButton)
				{
					//TODO: 서버 호출 함수로 변경
					ItemDragDrop->SourceInventory->Server_AddItemAmountAtSlot(ItemDragDrop->SourceIndex, ItemData.Amount);
				}
				return false;
			}
		}
		
		//우클릭이면 떨구기면 반갈한 것만 버리기
		if (InDragDropEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			UE_LOG(LogTemp, Warning, TEXT("RIGHT CLICK DROP"));
			//TODO: 서버 호출 함수로 변경
			PlayerRef->Request_DropItem(
				ItemDragDrop->SourceInventory, ItemDragDrop->SourceIndex, ItemData.Amount, false);
			return true;
		}
		
		//좌클릭 떨구기면 싹다 버리기
		UE_LOG(LogTemp, Warning, TEXT("LEFT CLICK DROP"));
		//TODO: 서버 호출 함수로 변경
		PlayerRef->Request_DropItem(
			ItemDragDrop->SourceInventory, ItemDragDrop->SourceIndex, ItemData.Amount, true);
		return true;
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("DROP ON PANEL ITEM INVALID"));
	}
	return false;
}

void UInventory::NativeDestruct()
{
	CraftRecipeButton->OnClicked.RemoveDynamic(this, &UInventory::HandleCraftRecipeClicked);
	FoodRecipeButton->OnClicked.RemoveDynamic(this, &UInventory::HandleFoodRecipeClicked);
	BuildingRecipeButton->OnClicked.RemoveDynamic(this, &UInventory::HandleBuildingRecipeClicked);
	UnknownRecordButton->OnClicked.RemoveDynamic(this, &UInventory::HandleUnknownRecordClicked);
	
	PlayerRef->StatusComponent->OnHPPercentChanged.RemoveDynamic(this, &UInventory::UpdateHP);
	PlayerRef->StatusComponent->OnStaminaPercentChanged.RemoveDynamic(this, &UInventory::UpdateStamina);
	PlayerRef->StatusComponent->OnHungerPercentChanged.RemoveDynamic(this, &UInventory::UpdateHunger);
	PlayerRef->StatusComponent->OnHydrationPercentChanged.RemoveDynamic(this, &UInventory::UpdateHydration);
	PlayerRef->StatusComponent->OnInfectionPercentChanged.RemoveDynamic(this, &UInventory::UpdateInfection);
	
	Super::NativeDestruct();
}

void UInventory::InventorySetting(AMainPlayer* Owner)
{
	if (Owner)
	{
		InventoryPanel->InventorySetting(Owner);
	}
}

void UInventory::UpdateHP(float NewHP)
{
	HPProgressBar->SetPercent(NewHP);
}

void UInventory::UpdateStamina(float NewStamina)
{
	StaminaProgressBar->SetPercent(NewStamina);
}

void UInventory::UpdateHunger(float NewHunger)
{
	HungerProgressBar->SetPercent(NewHunger);
}

void UInventory::UpdateHydration(float NewHydration)
{
	HydrationProgressBar->SetPercent(NewHydration);
}

void UInventory::UpdateInfection(float NewInfection)
{
	InfectionProgressBar->SetPercent(NewInfection);
}

void UInventory::HandleFoodRecipeClicked(UTextButton* ClickedButton)
{
	CurrentPanelButton->SetSelected(false);
	CurrentPanelButton = ClickedButton;
	CurrentPanelButton->SetSelected(true);
	
	PanelSwitcher->SetActiveWidget(FoodRecipeSection);
	if (UCraftPanel* FoodPanel = Cast<UCraftPanel>(FoodRecipeSection))
	{
		FoodPanel->SetCraftingMethod(ECraftMethod::INVEN);
	}
	OnCraftClicked.ExecuteIfBound();
}

void UInventory::HandleCraftRecipeClicked(UTextButton* ClickedButton)
{
	CurrentPanelButton->SetSelected(false);
	CurrentPanelButton = ClickedButton;
	CurrentPanelButton->SetSelected(true);
	
	PanelSwitcher->SetActiveWidget(CraftRecipeSection);
	if (UCraftPanel* CraftPanel = Cast<UCraftPanel>(CraftRecipeSection))
	{
		CraftPanel->SetCraftingMethod(ECraftMethod::INVEN);
	}
	OnCraftClicked.ExecuteIfBound();
}

void UInventory::HandleUnknownRecordClicked(UTextButton* ClickedButton)
{
	CurrentPanelButton->SetSelected(false);
	CurrentPanelButton = ClickedButton;
	CurrentPanelButton->SetSelected(true);
	
	PanelSwitcher->SetActiveWidget(UnknownRecordSection);
	if (UUnknownRecordPanel* UnknownPanel = Cast<UUnknownRecordPanel>(UnknownRecordSection))
	{
		UnknownPanel->RefreshList();
	}
	OnUnknownRecordClicked.ExecuteIfBound();
}

void UInventory::HandleBuildingRecipeClicked(UTextButton* ClickedButton)
{
	CurrentPanelButton->SetSelected(false);
	CurrentPanelButton = ClickedButton;
	CurrentPanelButton->SetSelected(true);
	
	PanelSwitcher->SetActiveWidget(BuildingRecipeSection);
	if (UBuildingPanel* BuildingPanel = Cast<UBuildingPanel>(BuildingRecipeSection))
	{
		BuildingPanel->SetBuildingMethod(ECraftMethod::INVEN);
	}
	
	OnCraftClicked.ExecuteIfBound();
}

