// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Games/MainGameState.h"
#include "ChattingPanel.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UChattingPanel : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	class UBorder* BackGround;
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	class UScrollBox* ChattingList;
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	class UEditableTextBox* ChattingInputBox;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class UChattingBlock> ChattingBlockClass;
	
	class AMainPlayerController* OwnedController;
	
	UPROPERTY(meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* OpenAnim;
	
	UPROPERTY(meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* CloseAnim;
	
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable)
	void FocusInput();
	
	UFUNCTION()
	void OnOpenAnimationFinished();
	
	UFUNCTION()
	void OnCloseAnimationFinished();
	
	UFUNCTION(BlueprintCallable)
	void ClearFocusInput();
	
	UFUNCTION(BlueprintCallable)
	void OnChattingCommited(const FText& Text, ETextCommit::Type CommitMethod);
	
	UFUNCTION(BlueprintCallable)
	void AddChatting(FChattingData NewChattingData);
};
