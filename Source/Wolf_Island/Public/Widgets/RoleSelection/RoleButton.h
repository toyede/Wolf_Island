// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Character/MainPlayer.h"
#include "RoleButton.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FRoleData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECharacterRole Role;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RoleName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RoleDescription;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnButtonClick, ECharacterRole, Role);

UCLASS()
class WOLF_ISLAND_API URoleButton : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnButtonClick OnClicked;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* RoleDataTable;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECharacterRole Role;
	
	UPROPERTY(meta=(BindWidget))
	class UButton* Button;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* RoleName;
	
	UPROPERTY(meta=(BindWidget))
	UTextBlock* RoleDesc;
	
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OnButtonClick();
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetOccupied(bool IsOccupied);
};
