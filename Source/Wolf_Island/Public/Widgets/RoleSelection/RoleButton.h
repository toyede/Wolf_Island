// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Character/MainPlayer.h"
#include "RoleButton.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnButtonClick, ECharacterRole, Role);

UCLASS()
class WOLF_ISLAND_API URoleButton : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(BlueprintAssignable)
	FOnButtonClick OnClicked;
	
	UPROPERTY(meta=(BindWidget))
	class UButton* Button;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECharacterRole Role;
	
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OnButtonClick();
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetOccupied(bool IsOccupied);
};
