// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RoleSelection.generated.h"

class UImage;
class UTextBlock;
class UBorder;
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
	UTextBlock* RoleName;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* RoleDesc;
	
	UPROPERTY(meta=(BindWidget))
	UWrapBox* RoleList;
	
	UPROPERTY(meta=(BindWidget))
	UImage* RoleThumbnail;
	
	UPROPERTY(meta=(BindWidget))
	UBaseButton* ConfirmButton;
	
	UPROPERTY(meta=(BindWidget))
	UBorder* AlarmBar;
	
	UPROPERTY(meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* DenyAlarm;
	
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void CheckOccupied();
	
	UFUNCTION(BlueprintNativeEvent)
	void SetInfoSection(ECharacterRole Role);
	
	UFUNCTION()
	void ConfirmSelection();
	
	UFUNCTION()
	void PlayDenyAlarm();
	
	UFUNCTION()
	void SetRandomRole();
};
