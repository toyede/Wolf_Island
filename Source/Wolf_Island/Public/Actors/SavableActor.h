// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Games/SaveInterface.h"
#include "SavableActor.generated.h"

UCLASS()
class WOLF_ISLAND_API ASavableActor : public AActor, public ISaveInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASavableActor();

protected:
	
	UPROPERTY(Replicated, SaveGame)
	FGuid GUID;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual FGuid GetGUID() const override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};
