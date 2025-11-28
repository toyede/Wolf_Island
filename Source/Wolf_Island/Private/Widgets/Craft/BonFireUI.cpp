#include "Widgets/Craft/BonFireUI.h"
#include "Widgets/Craft/CraftPanel.h"
#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "Input/Reply.h" 

void UBonFireUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 1. CraftPanel 설정 (화로 모드 + 음식)
	if (CraftPanel)
	{
		CraftPanel->RecipeTypeList.Empty();
		CraftPanel->RecipeTypeList.Add(EItemType::FOOD);
		CraftPanel->SetCraftingMethod(ECraftMethod::FIRE);
	}

	// 2. 닫기 버튼 클릭 이벤트 연결
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &UBonFireUI::HandleCloseClicked);
	}

	// 3. 입력 모드 설정 & 포커스 잡기
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetShowMouseCursor(true);

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(this->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        
		PC->SetInputMode(InputMode);
	}
	
	SetIsFocusable(true);
	SetKeyboardFocus();
}

void UBonFireUI::NativeDestruct()
{
	Super::NativeDestruct();

	// 위젯이 꺼질 때 게임 모드로 복귀
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
}

// [핵심] 키보드 눌림 감지
FReply UBonFireUI::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// 누른 키가 'Tab' 키 이거나 'Escape(ESC)' 키라면
	if (InKeyEvent.GetKey() == EKeys::Tab || InKeyEvent.GetKey() == EKeys::Escape)
	{
		HandleCloseClicked(); // 닫기 함수 실행
		return FReply::Handled(); // "내가 처리했으니 다른 애들은 신경 꺼" 라고 알림
	}

	return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}

void UBonFireUI::HandleCloseClicked()
{
	// 화면에서 제거 (자동으로 NativeDestruct 호출됨)
	RemoveFromParent();
}
