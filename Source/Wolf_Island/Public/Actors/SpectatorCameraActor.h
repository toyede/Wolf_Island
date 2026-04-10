// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpectatorCameraActor.generated.h"

class UCameraComponent;

UCLASS()
class WOLF_ISLAND_API ASpectatorCameraActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ASpectatorCameraActor();

	UPROPERTY(Replicated)
	AActor* TargetActor;

	float InterpSpeed = 15.0f;
	FVector Offset = FVector(-50.0f, 0.0f, 110.f); // 카메라 높이

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	USceneComponent* RootComp;
	UCameraComponent* CameraComp;
public:	
	virtual void Tick(float DeltaTime) override;

};
