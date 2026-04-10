// AnimalSpawnSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AnimalSpawnSubsystem.generated.h"

class AAnimalBase;

USTRUCT(BlueprintType)
struct FAnimalSpawnInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AAnimalBase> AnimalClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector SpawnCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnRadius = 3000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float InnerRadius = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RespawnDelay = 30.f;

    int32 CurrentCount = 0;
};

UCLASS()
class WOLF_ISLAND_API UAnimalSpawnSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Animal Spawn")
    void AddSpawnInfo(TSubclassOf<AAnimalBase> AnimalClass, FVector SpawnCenter,
        float SpawnRadius = 3000.f, float InnerRadius = 0.f,
        int32 MaxCount = 3, float RespawnDelay = 30.f);

    UFUNCTION(BlueprintCallable, Category = "Animal Spawn")
    void StartSpawning();

private:
    void SpawnAnimal(int32 InfoIndex);
    void HandleRespawn(int32 InfoIndex);
    FVector GetRandomNavMeshLocation(const FVector& Origin, float Radius, float InnerRadius);

    UFUNCTION()
    void OnAnimalDestroyed(AActor* DestroyedActor);

    TArray<FAnimalSpawnInfo> SpawnSettings;
    TMap<AActor*, int32> AnimalToIndexMap;
};