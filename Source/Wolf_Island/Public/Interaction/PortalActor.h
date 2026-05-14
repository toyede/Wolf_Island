// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableActor.h"
#include "PortalActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class AMainPlayer;
class AEnemyAIBoss;
class ULevelSequence;
class ULevelSequencePlayer;
class UMediaPlayer;
class UMediaSoundComponent;
class UMediaSource;
class UUserWidget;
class APortalActor;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPortalTriggered, APortalActor*, TriggeredPortal);

UCLASS()
class WOLF_ISLAND_API APortalActor : public AInteractableActor
{
	GENERATED_BODY()

public:
	APortalActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

	// Saved ID of the target portal (used to re-link after load)
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Portal", SaveGame)
	FString TargetPortalID;
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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void LoadData_Implementation(const FActorSaveData& InData) override;
	virtual void Interact_Implementation(AActor* Interactor) override;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnPortalTriggered OnPortalTriggered;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Portal")
	bool bRequiresBossDefeat = false;

	UPROPERTY(ReplicatedUsing = OnRep_BossDefeated)
	bool bBossDefeated = false;
	
	UPROPERTY(EditInstanceOnly, Category = "Boss")
	TSubclassOf<AEnemyAIBoss> BossClassToSpawn;

	UPROPERTY(EditInstanceOnly, Category = "Boss")
	FTransform BossSpawnTransform;

	UPROPERTY(EditInstanceOnly, Category = "Boss")
	TObjectPtr<APortalActor> ExitPortal;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Portal")
	TArray<AMainPlayer*> GetLastTriggeredPartyMembers() const;

	UFUNCTION()
	void OnRep_BossDefeated();

	UFUNCTION()
	void OnBossDefeated();
	
	UPROPERTY(EditInstanceOnly, Category = "Boss")
	AActor* BossSpawnPoint;

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
	FString ReadPortalIDFromActor(const AActor* Actor) const;
	FString GetPortalID() const;
	void ResolveTargetPortal();

	UPROPERTY()
	TSet<TWeakObjectPtr<AMainPlayer>> PlayersInVolume;

	UPROPERTY()
	TArray<TObjectPtr<AMainPlayer>> LastTriggeredPartyMembers;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media")
	UMediaPlayer* TeleportMediaPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media")
	UMediaSource* TeleportMediaSource;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Media")
	UMediaSoundComponent* MediaSoundComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media")
	TSubclassOf<UUserWidget> TeleportVideoWidgetClass;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayTeleportVideo();

protected:
	UFUNCTION()
	void OnTeleportVideoFinished();

private:
	UPROPERTY()
	UUserWidget* VideoWidgetInstance;
	UPROPERTY()
	ULevelSequencePlayer* SequencePlayer;
};
