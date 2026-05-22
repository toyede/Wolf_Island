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
class AController;
class AEnemyAIBase;

// ��Ƽ ���� �����ΰ� ���� ������
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

	// 늑대 액터가 먼저 사라져도 복귀 위치를 잃지 않기 위한 백업 Transform
	UPROPERTY()
	FTransform LastKnownWerewolfTransform = FTransform::Identity;

	// 변신 전 스탯 백업
	UPROPERTY()
	float BackupHP = 0.0f;

	UPROPERTY()
	float BackupStamina = 0.0f;

	UPROPERTY()
	float BackupHunger = 0.0f;

	UPROPERTY()
	float BackupHydration = 0.0f;

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
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

	// 하루 밤 달빛 노출량 누적 (매 밤 시작 시 0으로 초기화)
	UPROPERTY()
	TMap<TWeakObjectPtr<AMainPlayer>, float> NightlyExposure;

	// �̹� �㿡 �̹� Ʈ���ŵ� �÷��̾� (��� 1ȸ ����)
	UPROPERTY()
	TSet<TWeakObjectPtr<AMainPlayer>> TriggeredThisNight;

	// 밤 임계치 트리거 시점의 감염값 스냅샷 (아침 보너스 누적 기준)
	UPROPERTY()
	TMap<TWeakObjectPtr<AMainPlayer>, float> TriggeredInfectionSnapshot;

	UPROPERTY(EditAnywhere, Category = "Infection")
	float NightlyTransformThreshold = 15.0f; // �Ϸ� �� ���� 15% ������ Ʈ����

	UPROPERTY(EditAnywhere, Category = "Infection")
	float PostSequenceInfectionBonus = 20.0f; // Ʈ���� �� ������ ��ħ�� +20%

	UFUNCTION()
	void StartSingleInfectionSequence(AMainPlayer* Player);

	// BP���� ȣ���� �Լ�
	UFUNCTION(BlueprintCallable, Category = "Moonlight System")
	void OnNightStarted();

	UFUNCTION(BlueprintCallable, Category = "Moonlight System")
	void OnDayStarted();

	// 멀티에서 이번 밤 전원 감염(기절 제외) 여부를 재평가하고 필요 시 아침으로 스킵
	UFUNCTION(BlueprintCallable, Category = "Moonlight System")
	void EvaluateAllPlayersInfectedAndSkipMorning();

	// 멀티 Logout 시 내부 추적 데이터 정리
	void HandlePlayerLogout(AController* ExitingController);

	// ��Ƽ ����: �����ΰ� ���� ������
	UFUNCTION()
	void StartMultiInfectionSequence(AMainPlayer* Player);

	UFUNCTION()
	void OnMorningStarted();

	void RestorePlayerAtDawn(APlayerController* PC);

	// ����׿� - �����Ϳ��� ȣ���ϰų� Ű ���ε�
	UFUNCTION(BlueprintCallable, Category = "Moonlight System|Debug")
	void Debug_ForceRestoreAll();

	// 멀티 전용: 활성 늑대인간 변신 세션
	UPROPERTY()
	TMap<TWeakObjectPtr<APlayerController>, FWerewolfSessionData> ActiveWerewolfSessions;
private:
	UPROPERTY()
	TArray<TWeakObjectPtr<UStatusComponent>> InfectedStatusList;

	UPROPERTY(Transient)
	bool bMorningSkipTriggeredThisNight = false;

	UPROPERTY()
	AActor* DynamicSkyActor;

	UPROPERTY()
	ULightComponent* MoonLight;

	FTimerHandle CheckTimerHandle;

	void CheckAllPlayers();
	bool IsPlayerEligibleForNightSkip(AMainPlayer* Player) const;
	bool IsPlayerExposedToMoonlight(AActor* Player);
	void ApplyInfection(AActor* Player, float Amount);
	FVector GetMoonlightCheckLocation(AActor* Player);

	// ��Ƽ ����: �����ΰ� ���� ó��
	void SpawnAndPossessWerewolf(APlayerController* PC, FVector Location);
	void StoreOriginalCharacter(APlayerController* PC, AMainPlayer* Player);

	

	UPROPERTY(EditAnywhere, Category = "Infection|Multi")
	TSubclassOf<ACharacter> WerewolfClass;


	// ��Ƽ ����: ���� ���� ��ȯ
private:
	void SetSpectateTarget(APlayerController* PC);

public:
	void SwitchSpectateTarget(APlayerController* PC);


	// ��Ƽ ����: ���� ��忡�� ���� ĳ���ͷ� ����
public:
	void NotifyWerewolfDown(ACharacter* Werewolf);
};
