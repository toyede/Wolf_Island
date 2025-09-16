// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionInterface.generated.h"

//인터랙션 관련 데이터
USTRUCT(BlueprintType)
struct FInteractableData
{
	GENERATED_USTRUCT_BODY();

	FInteractableData() :
	Name(FText::GetEmpty()),
	Amount(0),
	InteractionDuration(3.0f),
	CanInteract(true)
	{

	};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Amount;

	//인터랙션에 필요한 시간 (몇 초간 누르고 있어야 하는 지)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InteractionDuration;

	//인터랙션 가능 상태 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool CanInteract;

};
// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class WOLF_ISLAND_API IInteractionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	FInteractableData InteractableData;
	
	virtual void BeginFocus();
	virtual void EndFocus();
	virtual void BeginInteract();
	virtual void EndInteract();
	virtual void Interact(AActor* Interactor);
};
