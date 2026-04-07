// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/InteractableActor.h"
#include "PortalActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class AMainPlayer;
class AEnemyAIBoss;

// 포탈 타서 이동했을 때의 이벤트 델리게이트	
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPortalTriggered);

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

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnPortalTriggered OnPortalTriggered;

	// 포탈마다 다름. 인스턴수 변수
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Portal")
	bool bRequiresBossDefeat = false;

	UPROPERTY(ReplicatedUsing = OnRep_BossDefeated)
	bool bBossDefeated = false;

	// 보스 스폰
	UPROPERTY(EditInstanceOnly, Category = "Boss")
	TSubclassOf<AEnemyAIBoss> BossClassToSpawn;

	UPROPERTY(EditInstanceOnly, Category = "Boss")
	FTransform BossSpawnTransform;

	UPROPERTY(EditInstanceOnly, Category = "Boss")
	TObjectPtr<APortalActor> ExitPortal;

	UPROPERTY()
	TObjectPtr<AEnemyAIBoss> SpawnedBoss;

	bool bBossSpawned = false;

	void SpawnAndStartBoss();

	UFUNCTION()
	void OnBossDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void OnRep_BossDefeated();

	UFUNCTION()
	void OnBossDefeated();

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
