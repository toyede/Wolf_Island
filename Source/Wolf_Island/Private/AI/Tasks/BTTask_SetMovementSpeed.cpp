// BTTask_SetMovementSpeed.cpp
#include "AI/Tasks/BTTask_SetMovementSpeed.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTTask_SetMovementSpeed::UBTTask_SetMovementSpeed()
{
	NodeName = TEXT("Set Movement Speed");
}

EBTNodeResult::Type UBTTask_SetMovementSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		return EBTNodeResult::Failed;
	}

	APawn* Pawn = AICon->GetPawn();
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return EBTNodeResult::Failed;
	}

	uint8 StateValue = BB->GetValueAsEnum(StateKey.SelectedKeyName);
	EEnemyState State = static_cast<EEnemyState>(StateValue);

	if (Pawn->Implements<UEnemyCommonInterface>())
	{
		IEnemyCommonInterface::Execute_SetMovementSpeed(Pawn, State);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}