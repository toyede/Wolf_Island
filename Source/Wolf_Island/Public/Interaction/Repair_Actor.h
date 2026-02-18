#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableActor.h"
#include "Data/ItemDataStruct.h"
#include "Engine/DataTable.h"
#include "Repair_Actor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRepairStatusChanged);

UCLASS()
class WOLF_ISLAND_API ARepair_Actor : public AInteractableActor
{
	GENERATED_BODY()

	virtual void BeginPlay() override;
	
public:

	ARepair_Actor();

	UPROPERTY(ReplicatedUsing = OnRep_CompletedRecipes, BlueprintReadOnly, Category = "Repair", SaveGame)
	TArray<FName> CompletedRecipeNames;

	UFUNCTION()
	void OnRep_CompletedRecipes();

	UPROPERTY(BlueprintAssignable, Category = "Repair")
	FOnRepairStatusChanged OnRepairStatusChanged;

	UPROPERTY(ReplicatedUsing = OnRep_RepairStatus, EditAnywhere, BlueprintReadWrite, Category = "Check", SaveGame)
	bool bIsBody = false;
	UPROPERTY(ReplicatedUsing = OnRep_RepairStatus, EditAnywhere, BlueprintReadWrite, Category = "Check", SaveGame)
	bool bIsEngine = false;
	UPROPERTY(ReplicatedUsing = OnRep_RepairStatus, EditAnywhere, BlueprintReadWrite, Category = "Check", SaveGame)
	bool bIsSteering = false;
	UPROPERTY(ReplicatedUsing = OnRep_RepairStatus, EditAnywhere, BlueprintReadWrite, Category = "Check", SaveGame)
	bool bIsRadar = false;
	UPROPERTY(ReplicatedUsing = OnRep_RepairStatus, EditAnywhere, BlueprintReadWrite, Category = "Check", SaveGame)
	bool bIsAnchor = false;
	
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool CheckBodyComplete();
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool CheckEngineComplete();
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool CheckSteeringComplete();
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool CheckRadarComplete();
	UFUNCTION(BlueprintCallable, Category = "Data")
	bool CheckAnchorComplete();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION(BlueprintCallable, Category = "Data")
	void CompleteRepair();
	
	UPROPERTY(EditDefaultsOnly)
	UDataTable* RepairRecipesTable;

	UPROPERTY()
	TMap<FName, bool> RepairStatusMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Repair|State")
	TMap<FName, FName> RecipeIDMap;

	UFUNCTION(BlueprintCallable, Category = "Repair")
	void MarkRecipeAsComplete(FName RecipeName);

	UFUNCTION(BlueprintCallable, Category = "Repair")
	bool IsRecipeComplete(FName TargetRecipeName);

	UFUNCTION(BlueprintCallable, Category = "Repair")
	void RestoreStateFromGameInstance();

	bool bHasLevelLoadStarted = false;

	UFUNCTION()
	void OnRep_RepairStatus();

	UFUNCTION(BlueprintImplementableEvent, Category = "Repair")
	void UpdateShipVisuals();
	
	//저장 관련 코드
	virtual void SaveData_Implementation(FActorSaveData& OutData) override;
	virtual void LoadData_Implementation(const FActorSaveData& InData) override;
};
