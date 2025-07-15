// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_ClawAttack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"

UBTT_ClawAttack::UBTT_ClawAttack()
{
	NodeName = TEXT("Claw Attack");
}

EBTNodeResult::Type UBTT_ClawAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC)
		return EBTNodeResult::Failed;
	ACharacter* AIChar = Cast<ACharacter>(AIC->GetPawn());
	if (!AIChar)
		return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
	if (!TargetActor)
		return EBTNodeResult::Failed;

	const float Distance = AIChar->GetDistanceTo(TargetActor);
	if (Distance > AttackRange)
		return EBTNodeResult::Failed;

	if (UAnimInstance* AnimInst = AIChar->GetMesh()->GetAnimInstance())
	{
		//if (AIChar && AIChar->AttackMontage)
	}
	return EBTNodeResult::Type();
}
