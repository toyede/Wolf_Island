// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintDataDefinitions.h"
#include "Blueprint/UserWidget.h"
#include "FriendBlock.generated.h"

class UImage;
/**
 * 
 */
class UTextBlock;

UCLASS()
class WOLF_ISLAND_API UFriendBlock : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBPFriendInfo FriendData;
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UTextBlock* PlayerName;
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UImage* PlayerIcon;	
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void UpdateBlock(const FBPFriendInfo& FriendInfo);
	void UpdateBlock_Implementation(const FBPFriendInfo& FriendInfo);
};
