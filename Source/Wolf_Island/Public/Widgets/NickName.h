// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NickName.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API UNickName : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(meta=(BindWidget), BlueprintReadWrite)
	UTextBlock* NickName;
	
};
