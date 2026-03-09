// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FishTrapScreen.generated.h"

UCLASS()
class WOLF_ISLAND_API UFishTrapScreen : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	class UFishTrapPanel* FishTrapPanel;

	UPROPERTY()
	class AMainPlayer* PlayerRef;

	void InitializeScreen(class AFishTrap* Trap, AActor* Interactor);

protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable)
	void CloseWidget();

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// Chest와 동일한 드래그 앤 드롭 버리기 지원
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	UPROPERTY()
	class AFishTrap* FishTrapRef;
};
