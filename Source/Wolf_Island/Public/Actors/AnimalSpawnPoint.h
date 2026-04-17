#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "AnimalSpawnPoint.generated.h"

class AAnimalBase;

/**
 * 동물이 스폰될 위치와 설정을 담는 액터입니다.
 * 레벨에 배치하여 직관적으로 스폰 지점을 관리할 수 있습니다.
 */
UCLASS()
class WOLF_ISLAND_API AAnimalSpawnPoint : public ATargetPoint
{
	GENERATED_BODY()

public:
	AAnimalSpawnPoint();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Spawn")
	TSubclassOf<AAnimalBase> AnimalClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Spawn")
	float SpawnRadius = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Spawn")
	float InnerRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Spawn")
	int32 MaxCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animal Spawn")
	float RespawnDelay = 30.f;
};
