// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChestScreen.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UChestScreen : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Chest")
	class AChest* ChestRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Chest")
	class AMainPlayer* PlayerRef;

	UPROPERTY(meta=(BindWidget))
	class UChestPanel* ChestPanel;

	UFUNCTION()
	void InitializeChest(AChest* Chest, AActor* Interactor);

protected:

	void CloseWidget();

	virtual void NativeConstruct() override;

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
};
