// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Interaction/InteractionInterface.h"
#include "RecordActor.generated.h"

UCLASS()
class WOLF_ISLAND_API ARecordActor : public AInteractableActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARecordActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record", meta = (GetOptions = "GetRecordID"), SaveGame)
	FString RecordID;

	UFUNCTION()
	TArray<FString> GetRecordID() const;

	virtual void Interact_Implementation(AActor* Interactor) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	//저장 관련 코드
	virtual void SaveData_Implementation(FActorSaveData& OutData) override;
	virtual void LoadData_Implementation(const FActorSaveData& InData) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Record")
	UDataTable* RecipeDataTable;

	UFUNCTION(CallInEditor)
	TArray<FString> GetRecipeNames() const;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record", meta = (GetOptions = "GetRecipeNames"))
	TArray<FName> SharedUnlockRecipes;
};
