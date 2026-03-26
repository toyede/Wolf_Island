// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ItemDataStruct.h"
#include "BuildingComponent.generated.h"

UENUM(BlueprintType)
enum class EBuildingState : uint8
{
	Idle,
	Placing,
	Building
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WOLF_ISLAND_API UBuildingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBuildingComponent();

	void EnterBuildMode(const FRecipeData& Recipe, const FBuildingData& BuildData);

	void ConfirmBuild();
	void CancelBuild();
	void SendDebugChat(FString Message);
	void RotatePreview(float AxisValue);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void FinishBuild();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestBuild(FRecipeData Recipe, FBuildingData BuildData, FTransform SpawnTransform);

	UFUNCTION(Server, Reliable)
	void Server_CancelBuild();

	void ExecuteSpawn(FRecipeData Recipe, FBuildingData BuildData, FTransform SpawnTransform);

	EBuildingState GetCurrentState() const { return CurrentState; }

	UFUNCTION(Server, Reliable)
	void Server_EnterBuildMode(FRecipeData Recipe, FBuildingData BuildData);

	UFUNCTION(BlueprintPure, Category = "Building")
	bool IsConstructing() const { return CurrentState == EBuildingState::Building; }

private:
	UPROPERTY(Replicated)
	EBuildingState CurrentState = EBuildingState::Idle;

	UPROPERTY(EditAnywhere, Category = "Building")
	float PreviewRotationStepDegrees = 15.0f;
    
	UPROPERTY(EditAnywhere, Category = "Building")
	TSubclassOf<AActor> PreviewClass;

	UPROPERTY()
	AActor* PreviewActor;

	FRecipeData CurrentRecipe;
	FBuildingData CurrentBuildData;
	FTimerHandle BuildTimerHandle;
	float BuildStartTime = 0.0f;
	float BuildDuration = 0.0f;

	void UpdatePreview();
	void UpdateBuildProgress();
	void CompleteBuild();
	bool CheckPlacementValid() const;
	FTransform AdjustSpawnTransformToGround(const FTransform& InTransform) const;
	void CleanupBuildMode();
};
