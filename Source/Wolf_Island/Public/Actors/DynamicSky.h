#pragma once

#include "CoreMinimal.h"
#include "Actors/SavableActor.h"
#include "Actors/Interfaces/SkyInterface.h"
#include "DynamicSky.generated.h"

class UDirectionalLightComponent;
class UExponentialHeightFogComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPostProcessComponent;
class USkyAtmosphereComponent;
class USkyLightComponent;
class UStaticMeshComponent;
class UVolumetricCloudComponent;
class UEclipseManagerComponent;
class FLifetimeProperty;
class AEnemyAIBase;
class AMoonlightInfectionSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDynamicSkyEvent);

UENUM(BlueprintType)
enum class EDynamicSkyCloudMode : uint8
{
	None UMETA(DisplayName = "None"),
	TwoDClouds UMETA(DisplayName = "2D Clouds"),
	VolumetricClouds UMETA(DisplayName = "Volumetric Clouds")
};

UENUM(BlueprintType)
enum class EDynamicSkyEclipseState : uint8
{
	None UMETA(DisplayName = "None"),
	Scheduled UMETA(DisplayName = "Scheduled"),
	InProgress UMETA(DisplayName = "In Progress"),
	Totality UMETA(DisplayName = "Totality"),
	Finished UMETA(DisplayName = "Finished")
};

UCLASS(Blueprintable)
class WOLF_ISLAND_API ADynamicSky : public ASavableActor, public ISkyInterface
{
	GENERATED_BODY()

public:
	ADynamicSky();

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SaveData_Implementation(FActorSaveData& OutData) override;
	virtual void LoadData_Implementation(const FActorSaveData& InData) override;

	virtual void RemoveEnemy_Implementation(AActor* EnemyActor) override;
	virtual void SkipToMorning_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = "DynamicSky")
	void InitializeDynamicMaterials();

	UFUNCTION(BlueprintCallable, Category = "DynamicSky")
	void RefreshSkyFromState();

	UFUNCTION(BlueprintCallable, Category = "DynamicSky|Time")
	void SetTimeOfDay(float NewTimeOfDay);

	UFUNCTION(BlueprintPure, Category = "DynamicSky|Time")
	float GetTimeSpeedHoursPerSecond() const;

	UFUNCTION(BlueprintCallable, Category = "DynamicSky|Enemies")
	void RegisterEnemy(AActor* EnemyActor);

	UFUNCTION(BlueprintCallable, Category = "DynamicSky|Clouds")
	void SetCloudMode(EDynamicSkyCloudMode NewCloudMode);

	UPROPERTY(BlueprintAssignable, Category = "DynamicSky|Events")
	FDynamicSkyEvent OnNightStarted;

	UPROPERTY(BlueprintAssignable, Category = "DynamicSky|Events")
	FDynamicSkyEvent OnDayStarted;

	UPROPERTY(BlueprintAssignable, Category = "DynamicSky|Events")
	FDynamicSkyEvent OnMorningSkipStarted;

	UPROPERTY(BlueprintAssignable, Category = "DynamicSky|Events")
	FDynamicSkyEvent OnMorningSkipFinished;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "DynamicSky|Events")
	FDynamicSkyEvent OnEclipseTotalityStarted;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "DynamicSky|Events")
	FDynamicSkyEvent OnEclipseTotalityEnded;

	//KSH-외부 블루프린트(BP_MainPlayer, BP_Native, BP_Clipping_Test)가 레퍼런스로 읽는 값이라 public에 둔다.
	//protected로 두면 UE가 외부 BP의 변수 접근을 막아 BP 컴파일 에러가 난다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_TimeOfDay, SaveGame, Category = "DynamicSky|Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float TimeOfDay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_DynamicTimeOfDay, SaveGame, Category = "DynamicSky|Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float DynamicTimeOfDay;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsCurrentlyNight, SaveGame, Category = "DynamicSky|Time")
	bool bIsCurrentlyNight;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DynamicSky|Components")
	USceneComponent* DefaultSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DynamicSky|Components")
	UDirectionalLightComponent* SunDirectionalLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DynamicSky|Components")
	UDirectionalLightComponent* MoonDirectionalLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DynamicSky|Components")
	USkyLightComponent* SkyLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DynamicSky|Components")
	USkyAtmosphereComponent* SkyAtmosphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DynamicSky|Components")
	UExponentialHeightFogComponent* ExponentialHeightFog;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DynamicSky|Components")
	UPostProcessComponent* PostProcess;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DynamicSky|Components")
	UStaticMeshComponent* SkySphereMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DynamicSky|Components")
	UVolumetricCloudComponent* VolumetricCloud;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DynamicSky|Components")
	UEclipseManagerComponent* EclipseManager;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DynamicSky|Materials")
	UMaterialInterface* SkySphereMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DynamicSky|Materials")
	UMaterialInterface* VolumetricCloudMaterial;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "DynamicSky|Materials")
	UMaterialInstanceDynamic* SkySphereMID;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "DynamicSky|Materials")
	UMaterialInstanceDynamic* VolumetricCloudMID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float DawnTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float DuskTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float MorningTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Time|Morning Skip", meta = (ClampMin = "0.0"))
	float FadeDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, SaveGame, Category = "DynamicSky|Time", meta = (ClampMin = "0.001", UIMin = "1.0"))
	float DayLengthMinutes;

	UPROPERTY(BlueprintReadOnly, Replicated, SaveGame, Category = "DynamicSky|Time")
	int32 DaysPassed;

	UPROPERTY(BlueprintReadOnly, Replicated, SaveGame, Category = "DynamicSky|Time")
	int32 CurrentDay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, SaveGame, Category = "DynamicSky|Time")
	bool bTimePaused;

	UPROPERTY(BlueprintReadOnly, Replicated, SaveGame, Category = "DynamicSky|Time")
	bool bMidnightTriggered;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|SunMoon")
	bool bControlSunMoonRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|SunMoon")
	bool bControlSunMoonVisibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|SunMoon|Visibility")
	bool bShouldShowSun;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|SunMoon|Visibility")
	bool bShouldShowMoon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|SunMoon|Visibility")
	bool bShouldShowStars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|SunMoon|Moon Light", meta = (ClampMin = "0.0"))
	float MoonLightIntensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|SunMoon|Moon Light")
	FLinearColor MoonLightColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|SunMoon|Moon Light", meta = (ClampMin = "0.0"))
	float MoonLightSourceAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|SunMoon|Moon Light", meta = (ClampMin = "0.0"))
	float MoonLightTemperature;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|SunMoon|Moon Light")
	bool bUseMoonLightTemperature;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Atmosphere|Day")
	FLinearColor DaytimeRayleighScattering;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Atmosphere|Day", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DaytimeMultiScatteringFactor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Atmosphere|Night")
	FLinearColor NighttimeRayleighScattering;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Atmosphere|Night", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float NighttimeMultiScatteringFactor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|SunMoon|Legacy", meta = (ToolTip = "Legacy interpolation value. The ver3 rotation path does not read this value."))
	float SunHorizonPitch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|SunMoon|Legacy", meta = (ToolTip = "Legacy interpolation value. The ver3 rotation path does not read this value."))
	float SunNoonPitch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|SunMoon|Ver3 Rotation")
	float SunPitchAtDawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|SunMoon|Ver3 Rotation")
	float SunPitchAtDusk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|SunMoon")
	float SunYaw;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|SunMoon")
	float MoonYaw;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|SunMoon|Ver3 Rotation")
	float SunRoll;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|SunMoon|Ver3 Rotation")
	float MoonRoll;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|SunMoon")
	FRotator SunRotationOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|SunMoon")
	FRotator MoonRotationOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Materials|Moon Billboard")
	bool bApplyMoonBillboardRotationStatic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Materials|Moon Billboard")
	bool bUseVer3MoonBillboardFlipFix;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Materials|Moon Billboard", meta = (ToolTip = "Moon pitch threshold used by the ver3-style moon billboard flip fix."))
	float MoonBillboardFlipThresholdPitch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Materials|Moon Billboard")
	bool bInvertMoonBillboardFlip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Materials|Moon Billboard")
	float MoonBillboardRotationBeforeZenith;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Materials|Moon Billboard")
	float MoonBillboardRotationAfterZenith;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Materials|Moon Billboard", meta = (ToolTip = "Fallback constant value used when the ver3-style flip fix is disabled."))
	float MoonBillboardRotationStatic;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "DynamicSky|Debug")
	float LastAppliedSunPitch;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "DynamicSky|Debug")
	float LastAppliedMoonPitch;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "DynamicSky|Debug")
	float LastAppliedMoonBillboardRotationStatic;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CloudAccumulatedTime, SaveGame, Category = "DynamicSky|Clouds")
	float CloudAccumulatedTime;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "DynamicSky|Clouds")
	float CloudSyncBaseTime;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "DynamicSky|Clouds")
	float CloudSyncBaseLocalTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Clouds", meta = (ClampMin = "0.05"))
	float CloudSyncInterval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentCloudMode, SaveGame, Category = "DynamicSky|Clouds")
	EDynamicSkyCloudMode CurrentCloudMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Clouds|2D", meta = (ClampMin = "0.01", DisplayName = "2D Clouds Tiling"))
	float TwoDCloudsTiling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Clouds|2D", meta = (DisplayName = "2D Clouds Panning Speed"))
	float TwoDCloudsPanningSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Clouds|2D", meta = (ClampMin = "0.0", DisplayName = "2D Clouds Brightness"))
	float TwoDCloudsBrightness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Clouds|2D", meta = (ClampMin = "0.0", ClampMax = "1.0", DisplayName = "2D Clouds Day Time Sky Tint Strength"))
	float TwoDCloudsDayTimeSkyTintStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Clouds|2D", meta = (ClampMin = "0.0", ClampMax = "1.0", DisplayName = "2D Clouds Night Time Sky Tint Strength"))
	float TwoDCloudsNightTimeSkyTintStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Clouds")
	float VolumCloudMovingSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Clouds")
	float VolumCloudLayerBottomAltitude;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Clouds")
	float VolumCloudLayerHeight;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_EclipseState, SaveGame, Category = "DynamicSky|Eclipse")
	EDynamicSkyEclipseState EclipseState;

	UPROPERTY(BlueprintReadOnly, Replicated, SaveGame, Category = "DynamicSky|Eclipse")
	bool bEclipseOccurred;

	UPROPERTY(BlueprintReadOnly, Replicated, SaveGame, Category = "DynamicSky|Eclipse")
	bool bEclipseTodayConfirmed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Eclipse")
	float EclipseMovingTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DynamicSky|Eclipse")
	float EclipseStopTime;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "DynamicSky|Enemies")
	TArray<AActor*> RegisteredEnemies;

	UPROPERTY(Transient)
	TObjectPtr<AMoonlightInfectionSystem> MoonlightInfectionSystem;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "DynamicSky|Night")
	bool bNightSystemActivated;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "DynamicSky|Time|Morning Skip")
	bool bMorningSkipInProgress;

	UFUNCTION()
	void OnRep_TimeOfDay();

	UFUNCTION()
	void OnRep_DynamicTimeOfDay();

	UFUNCTION()
	void OnRep_IsCurrentlyNight();

	UFUNCTION()
	void OnRep_CloudAccumulatedTime();

	UFUNCTION()
	void OnRep_CurrentCloudMode();

	UFUNCTION()
	void OnRep_EclipseState();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartMorningSkipFade();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFinishMorningSkipFade();

	UFUNCTION(BlueprintImplementableEvent, Category = "DynamicSky|Events")
	void BP_OnSkyInitialized();

	UFUNCTION(BlueprintImplementableEvent, Category = "DynamicSky|Events")
	void BP_OnSkyStateApplied();

	UFUNCTION(BlueprintImplementableEvent, Category = "DynamicSky|Events")
	void BP_OnSkyStateRestored();

	UFUNCTION(BlueprintImplementableEvent, Category = "DynamicSky|Events")
	void BP_OnNightStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "DynamicSky|Events")
	void BP_OnDayStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "DynamicSky|Events")
	void BP_OnMorningSkipStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "DynamicSky|Events")
	void BP_OnMorningSkipFinished();

	UFUNCTION(BlueprintImplementableEvent, Category = "DynamicSky|Events")
	void BP_OnEclipseStateChanged(EDynamicSkyEclipseState NewState);

	// Runs on the authoritative sky actor. BP_DynamicSky_ver4 should use this
	// event only to call its existing Reliable NetMulticast StartEclipse event.
	UFUNCTION(BlueprintImplementableEvent, Category = "DynamicSky|Eclipse")
	void BP_RequestEclipseStart();

	void AdvanceTime(float DeltaTime);
	void ApplySunMoonSettings();
	void ApplyDayNightEnvironmentSettings();
	void ApplySkyMaterialVisibilitySettings();
	void ApplyCloudComponentSettings();
	void ApplyCloudTimeToMaterial();
	void AdvanceCloudTime(float DeltaTime);
	void ResetCloudSyncBase();
	void UpdateDayNightState(bool bBroadcastEvents);
	void UpdateNightSystemActivation(bool bShouldActivate);
	AMoonlightInfectionSystem* ResolveMoonlightInfectionSystem();
	void RegisterExistingEnemies();
	void HandleActorSpawned(AActor* SpawnedActor);
	void ScheduleEnemyFormRefresh();
	void HandleScheduledEnemyFormRefresh();
	void ApplyEnemyFormsToRegisteredEnemies();
	void ApplyEnemyForm(AEnemyAIBase* EnemyActor) const;
	void HandleNewDay();
	void SynchronizeEclipseStateFromManager();
	void RestoreEclipseManagerFromSavedState();
	void SetEclipseState(EDynamicSkyEclipseState NewState);
	void TryStartScheduledEclipse();
	void CompleteMorningSkipAfterFadeOut();
	void CompleteMorningSkipAfterFadeIn();
	void ApplyMorningSkipCameraFade(float FromAlpha, float ToAlpha, bool bHoldWhenFinished);
	bool IsNightTime(float TestTimeOfDay) const;
	float CalculateCurrentCloudTime() const;
	float CalculateSunPitch(float TestTimeOfDay) const;
	float CalculateMoonBillboardRotationStatic(float TestMoonPitch) const;
	void EnsureSkyManagerTag();

	float CloudSyncUpdateElapsed;
	FDelegateHandle ActorSpawnedDelegateHandle;
	bool bEnemyFormRefreshScheduled;
	FTimerHandle MorningSkipFadeOutTimerHandle;
	FTimerHandle MorningSkipFadeInTimerHandle;
};
