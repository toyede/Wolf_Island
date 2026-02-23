// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RoleSelection.generated.h"

class UBaseButton;
class AMainPlayerController;
enum class ECharacterRole : uint8;
class UWrapBox;
class AMainGameState;
/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API URoleSelection : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	AMainPlayerController* PlayerController;
	
	UPROPERTY(BlueprintReadWrite)
	ECharacterRole SelectedRole;
	
	UPROPERTY(BlueprintReadWrite)
	AMainGameState* MainGameState;
	
	UPROPERTY(meta=(BindWidget))
	UWrapBox* RoleList;
	
	UPROPERTY(meta=(BindWidget))
	UBaseButton* ConfirmButton;
	
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void CheckOccupied();
	
	UFUNCTION(BlueprintNativeEvent)
	void SetInfoSection(ECharacterRole Role);
	
	UFUNCTION()
	void ConfirmSelection();
};
