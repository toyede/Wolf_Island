// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractionInterface.h"
#include "GameFramework/Actor.h"
#include "InteractableActor.generated.h"

UCLASS()
class WOLF_ISLAND_API AInteractableActor : public AActor , public IInteractionInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractableActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	//꾹 누르기 인터랙션 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InteractionDuration = 0.0f;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent)
	void Interact(AActor* Interactor) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetInteractionDuration() { return InteractableData.InteractionDuration; };

	//에디터 상에서만 실행
#if WITH_EDITOR
	//프로퍼티가 바뀌었을 때 실행되는 함수
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
