// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableActor.h"
#include "PortalActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class AMainPlayer;

UCLASS()
class WOLF_ISLAND_API APortalActor : public AInteractableActor
{
	GENERATED_BODY()

public:
	APortalActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	UStaticMeshComponent* ActiveMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	UStaticMeshComponent* InactiveMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	UBoxComponent* MultiReadyVolume;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Portal")
	AActor* TargetPortal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal", meta = (GetOptions = "GetRecordIDOptions"), SaveGame)
	FString RequiredRecordID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport")
	bool bUseRandomOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport", meta = (ClampMin = "0.0"))
	float RandomOffsetRadius = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport", meta = (ClampMin = "0.0"))
	float SpawnZOffset = 20.0f;

	UFUNCTION()
	TArray<FString> GetRecordIDOptions() const;

	virtual void BeginPlay() override;

	virtual void Interact_Implementation(AActor* Interactor) override;

protected:
	UFUNCTION()
	void HandleUnlockedRecordsChanged();

	UFUNCTION()
	void OnReadyVolumeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnReadyVolumeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	bool IsRecordUnlocked() const;
	void UpdatePortalState();
	bool AreAllPlayersInVolume() const;
	void TeleportAllPlayers();
	void TeleportPlayer(AActor* Interactor);

	UPROPERTY()
	TSet<TWeakObjectPtr<AMainPlayer>> PlayersInVolume;
};
