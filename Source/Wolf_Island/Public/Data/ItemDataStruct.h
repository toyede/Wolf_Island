#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "ItemDataStruct.generated.h"

class UItemBase;

//아이템 종류 이넘 클래스
UENUM(BlueprintType)
enum class EItemType : uint8
{
	REPAIR UMETA(DisplayName = "REPAIR"),
	FOOD UMETA(DisplayName = "FOOD"),
	MATERIAL UMETA(DisplayName = "MATERIAL"),
	FOOD_EQUIPMENT UMETA(DisplayName = "FOOD EQUIPMENT"),
	TRAP UMETA(DisplayName = "TRAP"),
	EQUIPMENT UMETA(DisplayName = "EQUIPMENT"),
	BUILDING UMETA(DisplayName = "BUILDING"),
};

//아이템 문자 데이터 (이름, 설명)
USTRUCT(BlueprintType)
struct FItemTextData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;
};

//아이템 숫자 데이터 (최대 수량, 한칸에 여러개 가능 여부)
USTRUCT(BlueprintType)
struct FItemNumericData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAmount = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Stamina = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Hydration = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Hunger = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Durability = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InteractionDuration = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsStackable = false;
};

//아이템 에셋 데이터 (아이콘, 메쉬, BP)
USTRUCT(BlueprintType)
struct FItemAssetData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* Mesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBlueprint* BPMesh = nullptr;
};

//아이템 데이터 (최종)
USTRUCT(BlueprintType)
struct FItemData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FName ID = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	EItemType Type = EItemType::MATERIAL;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FItemTextData TextData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FItemNumericData NumericData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	FItemAssetData AssetData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	TSubclassOf<class UItemBase> ItemClass = nullptr;
	
};

//슬롯 데이터
USTRUCT(BlueprintType)
struct FItemSlot
{
	GENERATED_USTRUCT_BODY();

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	UPROPERTY()
	TObjectPtr<class UItemBase> Item = nullptr;
};

//조합기 타입 (인벤토리, 화로 등)
UENUM(BlueprintType)
enum class ECraftMethod : uint8
{
	INVEN UMETA(DisplayName = "INVENTORY"),
	FIRE UMETA(DisplayName = "FIRE"),
};

//레시피 데이터
USTRUCT(BlueprintType)
struct FRecipeData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere)
	ECraftMethod Method = ECraftMethod::INVEN;
	UPROPERTY(EditAnywhere)
	EItemType ItemType = EItemType::MATERIAL;
	UPROPERTY(EditAnywhere)
	FName Ingredient1ID = NAME_None;
	UPROPERTY(EditAnywhere)
	int32 Ingredient1Amount = 0;
	UPROPERTY(EditAnywhere)
	FName Ingredient2ID = NAME_None;
	UPROPERTY(EditAnywhere)
	int32 Ingredient2Amount = 0;
	UPROPERTY(EditAnywhere)
	FName Ingredient3ID = NAME_None;
	UPROPERTY(EditAnywhere)
	int32 Ingredient3Amount = 0;
	UPROPERTY(EditAnywhere)
	FName ResultID = NAME_None;
	UPROPERTY(EditAnywhere)
	int32 ResultAmount = 0;
	UPROPERTY(EditAnywhere)
	float Duration = 0.0f;

	TArray<FName> GetIngredientsID() const
	{
		TArray<FName> Result;
		
		if (!Ingredient1ID.IsNone()) Result.Add(Ingredient1ID);
		if (!Ingredient2ID.IsNone()) Result.Add(Ingredient2ID);
		if (!Ingredient3ID.IsNone()) Result.Add(Ingredient3ID);

		return Result;
	}
	
	TMap<FName, int32> GetIngredients() const
	{
		TMap<FName, int32> Result;
		
		if (!Ingredient1ID.IsNone()) Result.Add(Ingredient1ID, Ingredient1Amount);
		if (!Ingredient2ID.IsNone()) Result.Add(Ingredient2ID, Ingredient2Amount);
		if (!Ingredient3ID.IsNone()) Result.Add(Ingredient3ID, Ingredient3Amount);

		return Result;
	}
};

//수리 레시피 데이터
USTRUCT(BlueprintType)
struct FRepairRecipeData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere)
	FName RecipeName = NAME_None;
	UPROPERTY(EditAnywhere)
	FName Ingredient1ID = NAME_None;
	UPROPERTY(EditAnywhere)
	int32 Ingredient1Amount = 0;
	UPROPERTY(EditAnywhere)
	FName Ingredient2ID;
	UPROPERTY(EditAnywhere)
	int32 Ingredient2Amount;
	UPROPERTY(EditAnywhere)
	FName Ingredient3ID;
	UPROPERTY(EditAnywhere)
	int32 Ingredient3Amount;
	UPROPERTY(EditAnywhere)
	FName Ingredient4ID;
	UPROPERTY(EditAnywhere)
	int32 Ingredient4Amount = 0;
	UPROPERTY(EditAnywhere)
	float Duration = 0.0f;
	UPROPERTY(EditAnywhere)
	bool Complete = false;

	TArray<FName> GetIngredientsID() const
	{
		TArray<FName> Result;
		
		if (!Ingredient1ID.IsNone()) Result.Add(Ingredient1ID);
		if (!Ingredient2ID.IsNone()) Result.Add(Ingredient2ID);
		if (!Ingredient3ID.IsNone()) Result.Add(Ingredient3ID);
		if (!Ingredient4ID.IsNone()) Result.Add(Ingredient4ID);

		return Result;
	}
	
	TMap<FName, int32> GetIngredients() const
	{
		TMap<FName, int32> Result;
		
		if (!Ingredient1ID.IsNone()) Result.Add(Ingredient1ID, Ingredient1Amount);
		if (!Ingredient2ID.IsNone()) Result.Add(Ingredient2ID, Ingredient2Amount);
		if (!Ingredient3ID.IsNone()) Result.Add(Ingredient3ID, Ingredient3Amount);
		if (!Ingredient4ID.IsNone()) Result.Add(Ingredient4ID, Ingredient4Amount);

		return Result;
	}
	
};


// 아이템 저장 구조체
USTRUCT(BlueprintType)
struct FSavedActorData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	FTransform ActorTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int32 ItemAmount;

	FSavedActorData()
	{
		ActorClass = nullptr;
		ActorTransform = FTransform::Identity;
		ItemID = NAME_None;
		ItemAmount = 1;
	}
};

// 아이템 저장 '배열' 구조체
USTRUCT(BlueprintType)
struct FSavedActorList
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	TArray<FSavedActorData> Actors;
};

//알 수 없는 기록 구조체
USTRUCT(BlueprintType)
struct FUnknownRecord : public FTableRowBase
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere)
	FName RecordID;
	UPROPERTY(EditAnywhere)
	FText RecordTitle;
	UPROPERTY(EditAnywhere)
	FText RecordContent;
	UPROPERTY(EditAnywhere)
	UTexture2D* RecordImage = nullptr;
};