// AnimalSpawnSubsystem.cpp
#include "Subsystem/AnimalSpawnSubsystem.h"
#include "NavigationSystem.h"
#include "AI/Animal/AnimalBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

bool UAnimalSpawnSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    UWorld* World = Cast<UWorld>(Outer);
    if (!World) return false;
    return World->GetNetMode() != NM_Client;
}

void UAnimalSpawnSubsystem::Deinitialize()
{
    GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
    SpawnSettings.Empty();
    AnimalToIndexMap.Empty();
    Super::Deinitialize();
}

void UAnimalSpawnSubsystem::AddSpawnInfo(TSubclassOf<AAnimalBase> AnimalClass, FVector SpawnCenter,
    float SpawnRadius, float InnerRadius, int32 MaxCount, float RespawnDelay)
{
    FAnimalSpawnInfo Info;
    Info.AnimalClass = AnimalClass;
    Info.SpawnCenter = SpawnCenter;
    Info.SpawnRadius = SpawnRadius;
	Info.InnerRadius = InnerRadius;
    Info.MaxCount = MaxCount;
    Info.RespawnDelay = RespawnDelay;
    SpawnSettings.Add(Info);
}

void UAnimalSpawnSubsystem::StartSpawning()
{
    for (int32 i = 0; i < SpawnSettings.Num(); i++)
    {
        for (int32 j = 0; j < SpawnSettings[i].MaxCount; j++)
        {
            SpawnAnimal(i);
        }
    }
}

void UAnimalSpawnSubsystem::SpawnAnimal(int32 InfoIndex)
{
    if (!SpawnSettings.IsValidIndex(InfoIndex)) return;

    FAnimalSpawnInfo& Info = SpawnSettings[InfoIndex];
    if (!Info.AnimalClass) return;
    if (Info.CurrentCount >= Info.MaxCount) return;

    FVector SpawnLoc = GetRandomNavMeshLocation(Info.SpawnCenter, Info.SpawnRadius, Info.InnerRadius);


    if (SpawnLoc.IsZero())
    {
        return;
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AAnimalBase* Animal = GetWorld()->SpawnActor<AAnimalBase>(Info.AnimalClass, SpawnLoc, FRotator::ZeroRotator, Params);
    if (Animal)
    {
        Info.CurrentCount++;
        AnimalToIndexMap.Add(Animal, InfoIndex);
        Animal->OnDestroyed.AddDynamic(this, &UAnimalSpawnSubsystem::OnAnimalDestroyed);
    }
}

void UAnimalSpawnSubsystem::OnAnimalDestroyed(AActor* DestroyedActor)
{
    int32* IndexPtr = AnimalToIndexMap.Find(DestroyedActor);
    if (!IndexPtr) return;

    int32 InfoIndex = *IndexPtr;
    AnimalToIndexMap.Remove(DestroyedActor);

    if (SpawnSettings.IsValidIndex(InfoIndex))
    {
        SpawnSettings[InfoIndex].CurrentCount--;
        HandleRespawn(InfoIndex);
    }
}

void UAnimalSpawnSubsystem::HandleRespawn(int32 InfoIndex)
{
    if (!SpawnSettings.IsValidIndex(InfoIndex)) return;

    FTimerHandle TimerHandle;
    FTimerDelegate Delegate;
    Delegate.BindUObject(this, &UAnimalSpawnSubsystem::SpawnAnimal, InfoIndex);

    GetWorld()->GetTimerManager().SetTimer(TimerHandle, Delegate,
        SpawnSettings[InfoIndex].RespawnDelay, false);
}

FVector UAnimalSpawnSubsystem::GetRandomNavMeshLocation(const FVector& Origin, float Radius, float InnerRadius)
{
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys) return FVector::ZeroVector;

    const int32 MaxAttempts = 10;
    for (int32 i = 0; i < MaxAttempts; i++)
    {
        FNavLocation Result;
        if (NavSys->GetRandomReachablePointInRadius(Origin, Radius, Result))
        {
            float Dist = FVector::Dist2D(Origin, Result.Location);
            if (Dist >= InnerRadius)
            {
                return Result.Location;
            }
        }
    }
    return FVector::ZeroVector;
}