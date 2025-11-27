// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Craft/RepairUI.h"
#include "Widgets/Craft/RepairPanel.h"
#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "Input/Reply.h" 

void URepairUI::NativeConstruct()
{
	Super::NativeConstruct();
	// 2. 닫기 버튼 클릭 이벤트 연결
	if (CloseButton)
	{
		CloseButton->OnClicked.AddDynamic(this, &URepairUI::HandleCloseClicked);
	}

	// 3. 입력 모드 설정 & 포커스 잡기
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetShowMouseCursor(true);
		
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(this->TakeWidget()); // 포커스를 이 위젯으로 고정
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        
		PC->SetInputMode(InputMode);
	}
	
	SetIsFocusable(true);
	SetKeyboardFocus();
}

void URepairUI::NativeDestruct()
{
	Super::NativeDestruct();

	// 위젯이 꺼질 때 게임 모드로 복귀
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
}

FReply URepairUI::NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// 누른 키가 'Tab' 키 이거나 'Escape(ESC)' 키라면
	if (InKeyEvent.GetKey() == EKeys::Tab || InKeyEvent.GetKey() == EKeys::Escape)
	{
		HandleCloseClicked(); // 닫기 함수 실행
		return FReply::Handled(); // "내가 처리했으니 다른 애들은 신경 꺼" 라고 알림
	}

	return Super::NativeOnKeyDown(MyGeometry, InKeyEvent);
}

void URepairUI::HandleCloseClicked()
{
	// 화면에서 제거 (자동으로 NativeDestruct 호출됨)
	RemoveFromParent();
}
