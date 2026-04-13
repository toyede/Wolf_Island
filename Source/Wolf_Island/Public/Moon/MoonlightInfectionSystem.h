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

	// �Ϸ� �� ������ ���� ������ (�� ���� �� 0���� �ʱ�ȭ)
	UPROPERTY()
	TMap<AMainPlayer*, float> NightlyExposure;

	// �̹� �㿡 �̹� Ʈ���ŵ� �÷��̾� (��� 1ȸ ����)
	UPROPERTY()
	TSet<TWeakObjectPtr<AMainPlayer>> TriggeredThisNight;

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

	// ��Ƽ ����: �����ΰ� ���� ������
	UFUNCTION()
	void StartMultiInfectionSequence(AMainPlayer* Player);

	UFUNCTION()
	void OnMorningStarted();

	void RestorePlayerAtDawn(APlayerController* PC);

	// ����׿� - �����Ϳ��� ȣ���ϰų� Ű ���ε�
	UFUNCTION(BlueprintCallable, Category = "Moonlight System|Debug")
	void Debug_ForceRestoreAll();

	// ��Ƽ ����: Ȱ�� �����ΰ� ���� ������
	UPROPERTY()
	TMap<APlayerController*, FWerewolfSessionData> ActiveWerewolfSessions;
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
