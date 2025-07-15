#include "WolfAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"

AWolfAIController::AWolfAIController()
{
    // Create Components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
}

void AWolfAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Pawn Type is Character and BT Asset have to exist
    
    if (BT_WolfAI && InPawn)
    {
        BlackboardComponent->InitializeBlackboard(*(BT_WolfAI->BlackboardAsset));

        BehaviorTreeComponent->StartTree(*BT_WolfAI);
    }
}
