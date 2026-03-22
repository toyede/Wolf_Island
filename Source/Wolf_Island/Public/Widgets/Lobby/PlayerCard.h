// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerCard.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UPlayerCard : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UTextBlock* PlayerName;
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UTextBlock* PlayerReady;
	
	UFUNCTION(BlueprintCallable)
	void SetReady(bool IsReady);
	
	UFUNCTION(BlueprintCallable)
	void UpdateCard(APlayerController* PlayerController);
	
	virtual void NativeConstruct() override;
};
