// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RecipeBlock.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemDataStruct.h"
#include "RepairBlock.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRepairBlockClicked, FName, RowName, FRepairRecipeData, RecipeData);

UCLASS()
class WOLF_ISLAND_API URepairBlock : public UUserWidget
{
	GENERATED_BODY()
	
	public:
		UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnRepairBlockClicked OnRepairBlockClicked;
	
		UPROPERTY(BlueprintReadOnly, Category = "Data")
		FName RowName;

		UPROPERTY(BlueprintReadOnly, Category = "Data")
		FRepairRecipeData RepairRecipeData;
    	
    	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    	class UTextBlock* RecipeName;
    	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    	class UButton* RecipeButton;
    
    	UFUNCTION()
    	void OnRepairButtonClicked();

		UPROPERTY(BlueprintReadOnly, Category = "Repair")
		class ARepair_Actor* TargetActor;

		UFUNCTION(BlueprintCallable, Category = "Repair")
		void RefreshBlockStatus();
    
    protected:
    	virtual void NativeConstruct() override;
};
