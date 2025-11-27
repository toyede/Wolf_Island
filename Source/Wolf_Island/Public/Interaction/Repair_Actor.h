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
public:
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
	
};
