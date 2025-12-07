#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemDataStruct.h"
#include "BonFireUI.generated.h"

UCLASS()
class WOLF_ISLAND_API UBonFireUI : public UUserWidget
{
	GENERATED_BODY()
    
public:
	// WBP의 CraftPanel 위젯
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	class UCraftPanel* CraftPanel;

	// WBP의 닫기 버튼
	UPROPERTY(VisibleAnywhere, meta=(BindWidget))
	class UButton* CloseButton;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    TObjectPtr<USoundBase> FireSound;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// [추가] 키보드 입력을 처리하는 함수 (탭 키 감지용)
	virtual FReply NativeOnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	UFUNCTION()
	void HandleCloseClicked();
};
