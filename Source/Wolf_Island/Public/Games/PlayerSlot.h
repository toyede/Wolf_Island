// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerSlot.generated.h"

class UArrowComponent;
enum class ECharacterRole : uint8;
class AMainPlayerState;

UCLASS()
class WOLF_ISLAND_API APlayerSlot : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlayerSlot();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent* DefaultSceneRoot;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UArrowComponent* ArrowComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	APlayerController* PlayerController;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AMainPlayerState* PlayerState;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	AActor* PlayerVisual;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<AActor>> PlayerVisualClasses;
	
	UFUNCTION(BlueprintCallable)
	void AddPlayer(APlayerController* NewPlayer);
	
	UFUNCTION(BlueprintCallable)
	void RemovePlayer();
	
	UFUNCTION(BlueprintCallable)
	void ChangeRole(ECharacterRole NewRole);
	
	UFUNCTION(BlueprintCallable)
	void RefreshSlot();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

};
