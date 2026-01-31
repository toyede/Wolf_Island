// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "Net/UnrealNetwork.h"
#include "AnimalController.generated.h"

class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;
class UAISenseConfig_Scent;
class UBehaviorTreeComponent;


UENUM(BlueprintType)
enum class EAnimalState : uint8
{
	None UMETA(DisplayName = "None"),
	Idle UMETA(DisplayName = "Idle"),
	Patrol UMETA(DisplayName = "Patrol"),
	Escaping UMETA(DisplayName = "Escaping"),
	Dead UMETA(DisplayName = "Dead")
};

UCLASS()
class WOLF_ISLAND_API AAnimalController : public AAIController
{
	GENERATED_BODY()
	
public:
	AAnimalController();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_State, Category = "AI|State")
	EAnimalState AnimalState;

	UFUNCTION(BlueprintCallable, Category = "AI|State")
	void SetAnimalState(EAnimalState NewState);

	UFUNCTION()
	void OnRep_State();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	//~ BehaviorTree
	UPROPERTY(EditAnywhere, Category = "AI|BehaviorTree")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|BehaviorTree")
	TObjectPtr<UBehaviorTreeComponent> BehaviorComp;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
	FName StateKey = TEXT("State");

	UPROPERTY(EditDefaultsOnly, Category = "AI|Blackboard")
	FName TargetKey = TEXT("Target");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;

	UFUNCTION()
	virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	// ÆÛ¼Á¼Ç
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Damage> DamageConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Scent> ScentConfig;

	void HandleSight(AActor* Actor, const FAIStimulus& Stimulus);
	void HandleDamage(AActor* Actor, const FAIStimulus& Stimulus);
	void HandleHearing(AActor* Actor, const FAIStimulus& Stimulus);
	void HandleScent(const FAIStimulus& Stimulus);
};
