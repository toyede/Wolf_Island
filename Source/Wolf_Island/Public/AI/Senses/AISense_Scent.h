#pragma once

#include "CoreMinimal.h"
#include "Perception/AISense.h"
#include "AISense_Scent.generated.h"

class UAISenseConfig_Scent;
class FGameplayDebuggerCategory;

USTRUCT(BlueprintType)
struct WOLF_ISLAND_API FAIScentEvent
{
    GENERATED_BODY()

    typedef UAISense_Scent FSenseClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TWeakObjectPtr<AActor> Instigator;

    float Age;

    FAIScentEvent()
        : Location(FVector::ZeroVector)
        , Intensity(1.f)
        , MaxRange(1000.f)
        , Instigator(nullptr)
        , Age(0.f)
    {
    }
};

UCLASS()
class WOLF_ISLAND_API UAISense_Scent : public UAISense
{
    GENERATED_BODY()

public:
    struct FDigestedScentProperties
    {
        float ScentDetectionRadius;
        bool bDisplayDebugSphere;

        FDigestedScentProperties();
        FDigestedScentProperties(const UAISenseConfig_Scent& SenseConfig);
    };

    TMap<FPerceptionListenerID, FDigestedScentProperties> DigestedProperties;

    UAISense_Scent();

    UFUNCTION(BlueprintCallable, Category = "AI|Perception", meta = (WorldContext = "WorldContextObject"))
    static void ReportScentEvent(UObject* WorldContextObject, const FVector& Location, float Intensity, float MaxRange, AActor* Instigator);

    void RegisterEvent(const FAIScentEvent& Event);

protected:
    virtual float Update() override;

    void OnNewListenerImpl(const FPerceptionListener& NewListener);

    void OnListenerRemovedImpl(const FPerceptionListener& UpdatedListener);

    UPROPERTY()
    TArray<FAIScentEvent> RegisteredEvents;

    UPROPERTY(EditDefaultsOnly, Category = "Sense")
    float ScentEventMaxAge;
};