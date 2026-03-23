#pragma once

#include "CoreMinimal.h"
#include "Perception/AISenseConfig.h"
#include "AI/Senses/AISense_Scent.h"
#include "AISenseConfig_Scent.generated.h"

UCLASS(meta = (DisplayName = "AI Scent Config"))
class WOLF_ISLAND_API UAISenseConfig_Scent : public UAISenseConfig
{
    GENERATED_BODY()

public:
    UAISenseConfig_Scent(const FObjectInitializer& ObjectInitializer);

    virtual TSubclassOf<UAISense> GetSenseImplementation() const override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sense", NoClear)
    TSubclassOf<UAISense_Scent> Implementation;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sense", meta = (UIMin = 0.0, ClampMin = 0.0))
    float ScentDetectionRadius;

#if WITH_GAMEPLAY_DEBUGGER
    virtual void DescribeSelfToGameplayDebugger(const UAIPerceptionComponent* PerceptionComponent, FGameplayDebuggerCategory* DebuggerCategory) const override;
#endif
};