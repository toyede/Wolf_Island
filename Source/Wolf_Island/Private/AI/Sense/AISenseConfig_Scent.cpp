#include "AI/Sense/AISenseConfig_Scent.h"
#include "Perception/AIPerceptionComponent.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerCategory.h"
#endif

UAISenseConfig_Scent::UAISenseConfig_Scent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    DebugColor = FColor::Purple;
    ScentDetectionRadius = 1000.f;

    Implementation = UAISense_Scent::StaticClass();
}

TSubclassOf<UAISense> UAISenseConfig_Scent::GetSenseImplementation() const
{
    return Implementation ? *Implementation : UAISense_Scent::StaticClass();
}

#if WITH_GAMEPLAY_DEBUGGER
void UAISenseConfig_Scent::DescribeSelfToGameplayDebugger(const UAIPerceptionComponent* PerceptionComponent,
    FGameplayDebuggerCategory* DebuggerCategory) const
{
    if (PerceptionComponent == nullptr || DebuggerCategory == nullptr)
    {
        return;
    }

    const AActor* BodyActor = PerceptionComponent->GetBodyActor();
    if (BodyActor != nullptr)
    {
        FVector BodyLocation, BodyFacing;
        PerceptionComponent->GetLocationAndDirection(BodyLocation, BodyFacing);

        DebuggerCategory->AddShape(FGameplayDebuggerShape::MakeCylinder(
            BodyLocation,
            ScentDetectionRadius,
            25.0f,
            DebugColor
        ));
    }
}
#endif