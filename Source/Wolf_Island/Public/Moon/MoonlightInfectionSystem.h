// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/LightComponent.h"
#include "GameFramework/Character.h"
#include "MoonlightInfectionSystem.generated.h"

class UStatusComponent;
class AMainPlayer;
class APlayerController;
class AEnemyAIBase;

// 멀티 전용 늑대인간 빙의 데이터
USTRUCT()
struct FWerewolfSessionData
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AMainPlayer> OriginalCharacter;

	UPROPERTY()
	TWeakObjectPtr<ACharacter> WerewolfCharacter;

	UPROPERTY()
	TWeakObjectPtr<APlayerController> OwningPC;

	bool bIsSpectating = false;
	bool bIsIncapacitated = false;
};

UCLASS()
class WOLF_ISLAND_API AMoonlightInfectionSystem : public AActor
{
	GENERATED_BODY()
	
public:	
	AMoonlightInfectionSystem();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moonlight Settings")
	float CheckInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moonlight Settings")
	float InfectionPerCheck = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moonlight Settings")
	float CapsuleRadius = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moonlight Settings")
	float CapsuleHalfHeight = 88.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moonlight Settings")
	float TraceDistance = 100000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDrawDebugTrace = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugMessages = true;

	UFUNCTION(BlueprintCallable, Category = "Moonlight System")
	void ActivateInfectionCheck();

	UFUNCTION(BlueprintCallable, Category = "Moonlight System")
	void DeactivateInfectionCheck();

	UFUNCTION()
	void BindPlayers(const TArray<AActor*>& Players);

	UFUNCTION()
	void HandleInfectionStarted(UStatusComponent* StatusComp);

	UPROPERTY(EditAnywhere, Category = "Infection|Single")
	float TransformThreshold = 0.15f; // 감염률이 이 값 이상이면 변신

	UPROPERTY(EditAnywhere, Category = "Infection|Single")
	float PostSequenceInfectionBonus = 0.2f; // 시퀀스 후 추가 감염률

	// 플레이어별 밤 누적 노출량
	TMap<AMainPlayer*, float> NightExposureAccumulated;

	// 밤당 1회만 발동하고 싶으면 사용
	TSet<TWeakObjectPtr<class AMainPlayer>> TriggeredThisNight;

	UFUNCTION()
	void StartSingleInfectionSequence(AMainPlayer* Player);

	UFUNCTION()
	void OnNightStarted();

	// 멀티 전용: 늑대인간 세션 데이터
	UFUNCTION()
	void StartMultiInfectionSequence(AMainPlayer* Player);

	UFUNCTION()
	void OnMorningStarted();

	void RestorePlayerAtDawn(APlayerController* PC);

	// 디버그용 - 에디터에서 호출하거나 키 바인딩
	UFUNCTION(BlueprintCallable, Category = "Moonlight System|Debug")
	void Debug_ForceRestoreAll();

private:
	UPROPERTY()
	TArray<TObjectPtr<UStatusComponent>> InfectedStatusList;

	UPROPERTY()
	AActor* DynamicSkyActor;

	UPROPERTY()
	ULightComponent* MoonLight;

	FTimerHandle CheckTimerHandle;

	void CheckAllPlayers();
	bool IsPlayerExposedToMoonlight(AActor* Player);
	void ApplyInfection(AActor* Player, float Amount);
	FVector GetMoonlightCheckLocation(AActor* Player);

	// 멀티 전용: 늑대인간 빙의 처리
	void SpawnAndPossessWerewolf(APlayerController* PC, FVector Location);
	void StoreOriginalCharacter(APlayerController* PC, AMainPlayer* Player);

	UPROPERTY()
	TMap<APlayerController*, FWerewolfSessionData> ActiveWerewolfSessions;

	UPROPERTY(EditAnywhere, Category = "Infection|Multi")
	TSubclassOf<ACharacter> WerewolfClass;


	// 멀티 전용: 관전 모드로 전환
private:
	void SetSpectateTarget(APlayerController* PC);
};
