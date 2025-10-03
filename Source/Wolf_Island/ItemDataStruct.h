#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemDataStruct.generated.h"

USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Sort;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Water;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Hungry;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Weight;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Durability;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Overlap;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBlueprint* BPMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
   	UTexture* UITexture;
};
