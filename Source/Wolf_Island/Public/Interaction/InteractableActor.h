// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractionInterface.h"
#include "Actors/SavableActor.h"
#include "GameFramework/Actor.h"
#include "InteractableActor.generated.h"

UCLASS()
class WOLF_ISLAND_API AInteractableActor : public ASavableActor, public IInteractionInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractableActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	//인터랙션 가능 여부
	UPROPERTY(ReplicatedUsing=OnRep_CanInteract, BlueprintReadWrite, EditAnywhere)
	bool CanInteract = true;
	
	//꾹 누르기 인터랙션 시간
	UPROPERTY(ReplicatedUsing=OnRep_IntaractionDuration, EditAnywhere, BlueprintReadWrite)
	float InteractionDuration = 0.0f;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent)
	void Interact(AActor* Interactor) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetInteractionDuration() { return InteractableData.InteractionDuration; };
	
	UFUNCTION(BlueprintCallable)
	void SetInteractionDuration(float NewInteractionDuration);
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnRep_CanInteract() { InteractableData.CanInteract = CanInteract; };
	
	UFUNCTION()
	void OnRep_IntaractionDuration() { InteractableData.InteractionDuration = InteractionDuration; };
};
