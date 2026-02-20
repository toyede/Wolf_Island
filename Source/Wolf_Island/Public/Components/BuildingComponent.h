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
	// 다른 플레이어에게도 건축 상태(애니메이션 등)를 보여주려면 Replicated 설정이 필요할 수 있습니다.
	UPROPERTY(Replicated)
	EBuildingState CurrentState = EBuildingState::Idle;
    
	UPROPERTY(EditAnywhere, Category = "Building")
	TSubclassOf<AActor> PreviewClass;

	UPROPERTY()
	AActor* PreviewActor;

	FRecipeData CurrentRecipe;
	FBuildingData CurrentBuildData;
	FTimerHandle BuildTimerHandle;

	void UpdatePreview();
	void CompleteBuild();
	bool CheckPlacementValid() const;
	void CleanupBuildMode();
};
