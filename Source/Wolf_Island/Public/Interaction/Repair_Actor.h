#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableActor.h"
#include "Data/ItemDataStruct.h"
#include "Engine/DataTable.h"
#include "Repair_Actor.generated.h"


UCLASS()
class WOLF_ISLAND_API ARepair_Actor : public AInteractableActor
{
	GENERATED_BODY()

	virtual void BeginPlay() override;
	
public:

	ARepair_Actor();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	bool bIsBody = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	bool bIsEngine = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	bool bIsSteering = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
	bool bIsRadar = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Check")
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

	UFUNCTION(BlueprintCallable, Category = "Repair|System")
	void RestoreStateFromGameInstance();
};
