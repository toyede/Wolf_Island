// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/InteractionInterface.h"
#include "RecordActor.generated.h"

UCLASS()
class WOLF_ISLAND_API ARecordActor : public AActor, public IInteractionInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARecordActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record", meta = (GetOptions = "GetRecordID"))
	FString RecordID;

	UFUNCTION()
	TArray<FString> GetRecordID() const;

	virtual void Interact(AActor* Interactor) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
