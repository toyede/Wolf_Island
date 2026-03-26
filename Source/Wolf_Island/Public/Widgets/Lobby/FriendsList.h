// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintDataDefinitions.h"
#include "Blueprint/UserWidget.h"
#include "FriendsList.generated.h"

class UScrollBox;
/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UFriendsList : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UScrollBox* FriendList;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void UpdateFriendList();
	void UpdateFriendList_Implementation();
};
