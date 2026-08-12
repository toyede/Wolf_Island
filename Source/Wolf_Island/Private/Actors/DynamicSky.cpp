#include "Actors/DynamicSky.h"

#include "AI/Enemy_Character/EnemyAIBase.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Moon/EclipseManagerComponent.h"
#include "Moon/MoonlightInfectionSystem.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

namespace DynamicSkyParameterNames
{
	static constexpr float FullDayHours = 24.0f;
	static constexpr float SecondsPerMinute = 60.0f;
	static constexpr float Ver3DayNightTransitionOffsetHours = 0.15f;

	static const FName IsMoonVisible(TEXT("IsMoonVisible"));
	static const FName IsStarVisible(TEXT("IsStarVisible"));
	static const FName IsSunVisible(TEXT("IsSunVisible"));
	static const FName Is2DCloudVisible(TEXT("Is2DCloudVisible"));
	static const FName TwoDCloudSettings(TEXT("2DCloudSettings"));
	static const FName MoonBillboardRotStatic(TEXT("MoonBillboardRotStatic"));
	static const FName CloudSyncTime(TEXT("CloudSyncTime"));
	static const FName PanningSpeed(TEXT("PanningSpeed"));

	float NormalizeTimeOfDay(float Value)
	{
		float NormalizedValue = FMath::Fmod(Value, FullDayHours);
		if (NormalizedValue < 0.0f)
		{
			NormalizedValue += FullDayHours;
		}

		return NormalizedValue;
	}

	float MapTimeOfDayUnclamped(float TimeOfDay, float RangeStart, float RangeEnd, float OutStart, float OutEnd)
	{
		float NormalizedTime = NormalizeTimeOfDay(TimeOfDay);
		const float NormalizedStart = NormalizeTimeOfDay(RangeStart);
		float NormalizedEnd = NormalizeTimeOfDay(RangeEnd);

		if (FMath::IsNearlyEqual(NormalizedStart, NormalizedEnd))
		{
			return OutStart;
		}

		if (NormalizedEnd <= NormalizedStart)
		{
			NormalizedEnd += FullDayHours;
		}

		if (NormalizedTime < NormalizedStart)
		{
			NormalizedTime += FullDayHours;
		}

		return FMath::GetMappedRangeValueUnclamped(
			FVector2D(NormalizedStart, NormalizedEnd),
			FVector2D(OutStart, OutEnd),
			NormalizedTime);
	}
}

ADynamicSky::ADynamicSky()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false);
	bAlwaysRelevant = true;
	SetNetUpdateFrequency(10.0f);
	SetMinNetUpdateFrequency(2.0f);

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	SetRootComponent(DefaultSceneRoot);

	SunDirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunDirectionalLight"));
	SunDirectionalLight->SetupAttachment(DefaultSceneRoot);
	SunDirectionalLight->SetIntensity(10.0f);
	SunDirectionalLight->SetLightSourceAngle(0.0f);
	SunDirectionalLight->SetAtmosphereSunLight(true);
	SunDirectionalLight->SetAtmosphereSunLightIndex(0);

	MoonDirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("MoonDirectionalLight"));
	MoonDirectionalLight->SetupAttachment(DefaultSceneRoot);
	MoonDirectionalLight->SetIntensity(1.0f);
	MoonDirectionalLight->SetLightSourceAngle(0.0f);
	MoonDirectionalLight->SetAtmosphereSunLight(true);
	MoonDirectionalLight->SetAtmosphereSunLightIndex(1);
	MoonDirectionalLight->SetVisibility(false);

	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(DefaultSceneRoot);
	SkyLight->SetIntensity(1.0f);

	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
	SkyAtmosphere->SetupAttachment(DefaultSceneRoot);

	ExponentialHeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("ExponentialHeightFog"));
	ExponentialHeightFog->SetupAttachment(DefaultSceneRoot);

	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	PostProcess->SetupAttachment(DefaultSceneRoot);

	SkySphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SM_SkySphere"));
	SkySphereMesh->SetupAttachment(DefaultSceneRoot);
	SkySphereMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkySphereMesh->SetGenerateOverlapEvents(false);

	VolumetricCloud = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricCloud"));
	VolumetricCloud->SetupAttachment(DefaultSceneRoot);

	EclipseManager = CreateDefaultSubobject<UEclipseManagerComponent>(TEXT("EclipseManager"));

	TimeOfDay = 8.0f;
	DynamicTimeOfDay = 8.0f;
	DawnTime = 6.0f;
	DuskTime = 18.0f;
	MorningTime = 6.0f;
	FadeDuration = 1.5f;
	DayLengthMinutes = 24.0f;
	DaysPassed = 0;
	CurrentDay = 0;
	bTimePaused = false;
	bIsCurrentlyNight = false;
	bMidnightTriggered = false;
	MoonlightInfectionSystem = nullptr;
	bNightSystemActivated = false;
	bEnemyFormRefreshScheduled = false;
	bMorningSkipInProgress = false;

	bControlSunMoonRotation = true;
	bControlSunMoonVisibility = true;
	bShouldShowSun = true;
	bShouldShowMoon = true;
	bShouldShowStars = true;
	MoonLightIntensity = 1.0f;
	MoonLightColor = FLinearColor(193.0f, 193.0f, 193.0f, 255.0f);
	MoonLightSourceAngle = 0.0f;
	MoonLightTemperature = 8000.0f;
	bUseMoonLightTemperature = true;
	DaytimeRayleighScattering = FLinearColor(0.175287f, 0.409607f, 1.0f, 30.211479f);
	DaytimeMultiScatteringFactor = 1.0f;
	NighttimeRayleighScattering = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
	NighttimeMultiScatteringFactor = 0.5f;
	SunHorizonPitch = 0.0f;
	SunNoonPitch = -90.0f;
	SunPitchAtDawn = -180.0f;
	SunPitchAtDusk = 0.0f;
	SunYaw = 180.0f;
	MoonYaw = 0.0f;
	SunRoll = -180.0f;
	MoonRoll = 0.0f;
	SunRotationOffset = FRotator::ZeroRotator;
	MoonRotationOffset = FRotator::ZeroRotator;
	bApplyMoonBillboardRotationStatic = true;
	bUseVer3MoonBillboardFlipFix = true;
	MoonBillboardFlipThresholdPitch = -90.0f;
	bInvertMoonBillboardFlip = false;
	MoonBillboardRotationBeforeZenith = 3.14f;
	MoonBillboardRotationAfterZenith = 0.0f;
	MoonBillboardRotationStatic = 3.14f;
	LastAppliedSunPitch = 0.0f;
	LastAppliedMoonPitch = 0.0f;
	LastAppliedMoonBillboardRotationStatic = 0.0f;

	CloudAccumulatedTime = 0.0f;
	CloudSyncBaseTime = 0.0f;
	CloudSyncBaseLocalTime = 0.0f;
	CloudSyncInterval = 2.0f;
	CloudSyncUpdateElapsed = 0.0f;
	CurrentCloudMode = EDynamicSkyCloudMode::None;
	TwoDCloudsTiling = 1.0f;
	TwoDCloudsPanningSpeed = 1.0f;
	TwoDCloudsBrightness = 1.0f;
	TwoDCloudsDayTimeSkyTintStrength = 0.1f;
	TwoDCloudsNightTimeSkyTintStrength = 0.9f;
	VolumCloudMovingSpeed = 0.5f;
	VolumCloudLayerBottomAltitude = 7.0f;
	VolumCloudLayerHeight = 8.0f;

	EclipseState = EDynamicSkyEclipseState::None;
	bEclipseOccurred = false;
	bEclipseTodayConfirmed = false;
	EclipseMovingTime = 20.0f;
	EclipseStopTime = 60.0f;

	EnsureSkyManagerTag();
}

void ADynamicSky::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	EnsureSkyManagerTag();
	InitializeDynamicMaterials();
	ResetCloudSyncBase();
	UpdateDayNightState(false);
	ApplySunMoonSettings();
	ApplyDayNightEnvironmentSettings();
	ApplySkyMaterialVisibilitySettings();
	ApplyCloudComponentSettings();
}

void ADynamicSky::BeginPlay()
{
	Super::BeginPlay();

	EnsureSkyManagerTag();
	InitializeDynamicMaterials();
	ResetCloudSyncBase();
	UpdateDayNightState(false);

	if (HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			ActorSpawnedDelegateHandle = World->AddOnActorSpawnedHandler(
				FOnActorSpawned::FDelegate::CreateUObject(this, &ADynamicSky::HandleActorSpawned));
		}

		RegisterExistingEnemies();
		UpdateNightSystemActivation(bIsCurrentlyNight);
	}

	RefreshSkyFromState();
	BP_OnSkyInitialized();
}

void ADynamicSky::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (ActorSpawnedDelegateHandle.IsValid())
		{
			World->RemoveOnActorSpawnedHandler(ActorSpawnedDelegateHandle);
			ActorSpawnedDelegateHandle.Reset();
		}

		World->GetTimerManager().ClearAllTimersForObject(this);
	}

	bEnemyFormRefreshScheduled = false;
	bMorningSkipInProgress = false;
	Super::EndPlay(EndPlayReason);
}

void ADynamicSky::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		AdvanceTime(DeltaTime);
		AdvanceCloudTime(DeltaTime);
	}

	ApplyCloudTimeToMaterial();
}

void ADynamicSky::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADynamicSky, TimeOfDay);
	DOREPLIFETIME(ADynamicSky, DynamicTimeOfDay);
	DOREPLIFETIME(ADynamicSky, DayLengthMinutes);
	DOREPLIFETIME(ADynamicSky, DaysPassed);
	DOREPLIFETIME(ADynamicSky, CurrentDay);
	DOREPLIFETIME(ADynamicSky, bTimePaused);
	DOREPLIFETIME(ADynamicSky, bIsCurrentlyNight);
	DOREPLIFETIME(ADynamicSky, bMidnightTriggered);
	DOREPLIFETIME(ADynamicSky, CloudAccumulatedTime);
	DOREPLIFETIME(ADynamicSky, CurrentCloudMode);
	DOREPLIFETIME(ADynamicSky, EclipseState);
	DOREPLIFETIME(ADynamicSky, bEclipseOccurred);
	DOREPLIFETIME(ADynamicSky, bEclipseTodayConfirmed);
}

void ADynamicSky::SaveData_Implementation(FActorSaveData& OutData)
{
	if (HasAuthority())
	{
		CloudAccumulatedTime = CalculateCurrentCloudTime();
		ResetCloudSyncBase();
	}

	Super::SaveData_Implementation(OutData);
}

void ADynamicSky::LoadData_Implementation(const FActorSaveData& InData)
{
	Super::LoadData_Implementation(InData);

	EnsureSkyManagerTag();
	InitializeDynamicMaterials();
	ResetCloudSyncBase();
	UpdateDayNightState(false);
	RefreshSkyFromState();
	BP_OnSkyStateRestored();

	if (HasAuthority())
	{
		ForceNetUpdate();
	}
}

void ADynamicSky::RemoveEnemy_Implementation(AActor* EnemyActor)
{
	if (!HasAuthority() || !EnemyActor)
	{
		return;
	}

	RegisteredEnemies.Remove(EnemyActor);
}

void ADynamicSky::SkipToMorning_Implementation()
{
	if (!HasAuthority() || bMorningSkipInProgress)
	{
		return;
	}

	bMorningSkipInProgress = true;
	bTimePaused = true;

	OnMorningSkipStarted.Broadcast();
	MulticastStartMorningSkipFade();
	ForceNetUpdate();

	const float ClampedFadeDuration = FMath::Max(FadeDuration, 0.0f);
	if (ClampedFadeDuration <= KINDA_SMALL_NUMBER)
	{
		CompleteMorningSkipAfterFadeOut();
		return;
	}

	GetWorldTimerManager().SetTimer(
		MorningSkipFadeOutTimerHandle,
		this,
		&ADynamicSky::CompleteMorningSkipAfterFadeOut,
		ClampedFadeDuration,
		false);
}

void ADynamicSky::CompleteMorningSkipAfterFadeOut()
{
	if (!HasAuthority() || !bMorningSkipInProgress)
	{
		return;
	}

	if (TimeOfDay > MorningTime)
	{
		++DaysPassed;
		++CurrentDay;
	}

	TimeOfDay = DynamicSkyParameterNames::NormalizeTimeOfDay(MorningTime);
	DynamicTimeOfDay = TimeOfDay;
	bMidnightTriggered = false;

	UpdateDayNightState(true);
	RefreshSkyFromState();

	ForceNetUpdate();

	OnMorningSkipFinished.Broadcast();
	MulticastFinishMorningSkipFade();

	const float ClampedFadeDuration = FMath::Max(FadeDuration, 0.0f);
	if (ClampedFadeDuration <= KINDA_SMALL_NUMBER)
	{
		CompleteMorningSkipAfterFadeIn();
		return;
	}

	GetWorldTimerManager().SetTimer(
		MorningSkipFadeInTimerHandle,
		this,
		&ADynamicSky::CompleteMorningSkipAfterFadeIn,
		ClampedFadeDuration,
		false);
}

void ADynamicSky::CompleteMorningSkipAfterFadeIn()
{
	if (!HasAuthority() || !bMorningSkipInProgress)
	{
		return;
	}

	bTimePaused = false;
	bMorningSkipInProgress = false;
	ForceNetUpdate();
}

void ADynamicSky::MulticastStartMorningSkipFade_Implementation()
{
	ApplyMorningSkipCameraFade(0.0f, 1.0f, true);
	BP_OnMorningSkipStarted();
}

void ADynamicSky::MulticastFinishMorningSkipFade_Implementation()
{
	ApplyMorningSkipCameraFade(1.0f, 0.0f, false);
	BP_OnMorningSkipFinished();
}

void ADynamicSky::ApplyMorningSkipCameraFade(float FromAlpha, float ToAlpha, bool bHoldWhenFinished)
{
	APlayerCameraManager* PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (!IsValid(PlayerCameraManager))
	{
		return;
	}

	PlayerCameraManager->StartCameraFade(
		FromAlpha,
		ToAlpha,
		FMath::Max(FadeDuration, 0.0f),
		FLinearColor::Black,
		true,
		bHoldWhenFinished);
}

void ADynamicSky::InitializeDynamicMaterials()
{
	SkySphereMID = nullptr;
	if (SkySphereMesh)
	{
		UMaterialInterface* SourceMaterial = SkySphereMesh->GetMaterial(0);
		if (!SourceMaterial)
		{
			SourceMaterial = SkySphereMaterial;
		}

		if (UMaterialInstanceDynamic* ExistingMID = Cast<UMaterialInstanceDynamic>(SourceMaterial))
		{
			SkySphereMID = ExistingMID;
		}
		else if (SourceMaterial)
		{
			SkySphereMID = SkySphereMesh->CreateDynamicMaterialInstance(0, SourceMaterial);
		}
	}

	VolumetricCloudMID = nullptr;
	if (VolumetricCloud)
	{
		UMaterialInterface* SourceMaterial = VolumetricCloud->GetMaterial();
		if (!SourceMaterial)
		{
			SourceMaterial = VolumetricCloudMaterial;
		}

		if (UMaterialInstanceDynamic* ExistingMID = Cast<UMaterialInstanceDynamic>(SourceMaterial))
		{
			VolumetricCloudMID = ExistingMID;
		}
		else if (SourceMaterial)
		{
			VolumetricCloudMID = UMaterialInstanceDynamic::Create(SourceMaterial, this);
			VolumetricCloud->SetMaterial(VolumetricCloudMID);
		}
	}
}

void ADynamicSky::RefreshSkyFromState()
{
	ApplySunMoonSettings();
	ApplyDayNightEnvironmentSettings();
	ApplySkyMaterialVisibilitySettings();
	ApplyCloudComponentSettings();
	BP_OnSkyStateApplied();
}

void ADynamicSky::SetTimeOfDay(float NewTimeOfDay)
{
	if (!HasAuthority())
	{
		return;
	}

	TimeOfDay = DynamicSkyParameterNames::NormalizeTimeOfDay(NewTimeOfDay);
	DynamicTimeOfDay = TimeOfDay;
	UpdateDayNightState(true);
	RefreshSkyFromState();
	ForceNetUpdate();
}

float ADynamicSky::GetTimeSpeedHoursPerSecond() const
{
	if (DayLengthMinutes <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	return DynamicSkyParameterNames::FullDayHours / (DayLengthMinutes * DynamicSkyParameterNames::SecondsPerMinute);
}

void ADynamicSky::RegisterEnemy(AActor* EnemyActor)
{
	if (!HasAuthority() || !IsValid(Cast<AEnemyAIBase>(EnemyActor)))
	{
		return;
	}

	RegisteredEnemies.AddUnique(EnemyActor);

	if (HasActorBegunPlay())
	{
		ScheduleEnemyFormRefresh();
	}
}

void ADynamicSky::SetCloudMode(EDynamicSkyCloudMode NewCloudMode)
{
	if (!HasAuthority())
	{
		return;
	}

	CurrentCloudMode = NewCloudMode;
	RefreshSkyFromState();
	ForceNetUpdate();
}

void ADynamicSky::OnRep_TimeOfDay()
{
	UpdateDayNightState(false);
	RefreshSkyFromState();
}

void ADynamicSky::OnRep_DynamicTimeOfDay()
{
	UpdateDayNightState(false);
	RefreshSkyFromState();
}

void ADynamicSky::OnRep_IsCurrentlyNight()
{
	RefreshSkyFromState();
}

void ADynamicSky::OnRep_CloudAccumulatedTime()
{
	ResetCloudSyncBase();
	RefreshSkyFromState();
}

void ADynamicSky::OnRep_CurrentCloudMode()
{
	RefreshSkyFromState();
}

void ADynamicSky::OnRep_EclipseState()
{
	BP_OnEclipseStateChanged(EclipseState);
	RefreshSkyFromState();
}

void ADynamicSky::AdvanceTime(float DeltaTime)
{
	if (bTimePaused || DeltaTime <= 0.0f)
	{
		return;
	}

	const float HoursPerSecond = GetTimeSpeedHoursPerSecond();
	if (HoursPerSecond <= 0.0f)
	{
		return;
	}

	const float PreviousTime = TimeOfDay;
	const float AdvancedTime = TimeOfDay + (DeltaTime * HoursPerSecond);
	const int32 WrappedDays = FMath::FloorToInt(AdvancedTime / DynamicSkyParameterNames::FullDayHours);

	TimeOfDay = DynamicSkyParameterNames::NormalizeTimeOfDay(AdvancedTime);
	DynamicTimeOfDay = TimeOfDay;

	if (WrappedDays > 0)
	{
		DaysPassed += WrappedDays;
		CurrentDay += WrappedDays;
		bMidnightTriggered = true;
	}
	else if (TimeOfDay >= PreviousTime)
	{
		bMidnightTriggered = false;
	}

	UpdateDayNightState(true);
	RefreshSkyFromState();
}

void ADynamicSky::ApplySunMoonSettings()
{
	const float SunPitch = CalculateSunPitch(TimeOfDay);
	const float MoonPitch = -SunPitch;
	LastAppliedSunPitch = SunPitch;
	LastAppliedMoonPitch = MoonPitch;

	if (bControlSunMoonRotation)
	{
		const FRotator SunRotation = FRotator(SunPitch, SunYaw, SunRoll) + SunRotationOffset;
		const FRotator MoonRotation = FRotator(MoonPitch, MoonYaw, MoonRoll) + MoonRotationOffset;

		if (SunDirectionalLight)
		{
			SunDirectionalLight->SetWorldRotation(SunRotation);
		}

		if (MoonDirectionalLight)
		{
			MoonDirectionalLight->SetWorldRotation(MoonRotation);
		}
	}

	if (SkySphereMID && bApplyMoonBillboardRotationStatic)
	{
		const float AppliedMoonBillboardRotationStatic = bUseVer3MoonBillboardFlipFix
			? CalculateMoonBillboardRotationStatic(MoonPitch)
			: MoonBillboardRotationStatic;

		LastAppliedMoonBillboardRotationStatic = AppliedMoonBillboardRotationStatic;
		SkySphereMID->SetScalarParameterValue(DynamicSkyParameterNames::MoonBillboardRotStatic, AppliedMoonBillboardRotationStatic);
	}

	if (bControlSunMoonVisibility)
	{
		const bool bShowMoonLight = bIsCurrentlyNight && bShouldShowMoon;
		const bool bShowSunLight = !bIsCurrentlyNight && bShouldShowSun;

		if (SunDirectionalLight)
		{
			SunDirectionalLight->SetVisibility(bShowSunLight, true);
			SunDirectionalLight->SetHiddenInGame(!bShowSunLight);
		}

		if (MoonDirectionalLight)
		{
			MoonDirectionalLight->SetVisibility(bShowMoonLight, true);
			MoonDirectionalLight->SetHiddenInGame(!bShowMoonLight);
		}
	}
}

void ADynamicSky::ApplyDayNightEnvironmentSettings()
{
	if (MoonDirectionalLight)
	{
		MoonDirectionalLight->SetUseTemperature(bUseMoonLightTemperature);

		if (bIsCurrentlyNight)
		{
			MoonDirectionalLight->SetIntensity(MoonLightIntensity);
			MoonDirectionalLight->SetLightColor(MoonLightColor);
			MoonDirectionalLight->SetLightSourceAngle(MoonLightSourceAngle);
			MoonDirectionalLight->SetTemperature(MoonLightTemperature);
		}
	}

	if (SkyAtmosphere)
	{
		SkyAtmosphere->SetRayleighScattering(
			bIsCurrentlyNight ? NighttimeRayleighScattering : DaytimeRayleighScattering);
		SkyAtmosphere->SetMultiScatteringFactor(
			bIsCurrentlyNight ? NighttimeMultiScatteringFactor : DaytimeMultiScatteringFactor);
	}
}

void ADynamicSky::ApplySkyMaterialVisibilitySettings()
{
	if (!SkySphereMID || !bControlSunMoonVisibility)
	{
		return;
	}

	const bool bShowSun = !bIsCurrentlyNight && bShouldShowSun;
	const bool bShowMoon = bIsCurrentlyNight && bShouldShowMoon;
	const bool bShowStars = bIsCurrentlyNight && bShouldShowStars;

	SkySphereMID->SetScalarParameterValue(DynamicSkyParameterNames::IsSunVisible, bShowSun ? 1.0f : 0.0f);
	SkySphereMID->SetScalarParameterValue(DynamicSkyParameterNames::IsMoonVisible, bShowMoon ? 1.0f : 0.0f);
	SkySphereMID->SetScalarParameterValue(DynamicSkyParameterNames::IsStarVisible, bShowStars ? 1.0f : 0.0f);
}

void ADynamicSky::ApplyCloudComponentSettings()
{
	const bool bShow2DClouds = CurrentCloudMode == EDynamicSkyCloudMode::TwoDClouds;
	if (SkySphereMID)
	{
		SkySphereMID->SetScalarParameterValue(
			DynamicSkyParameterNames::Is2DCloudVisible,
			bShow2DClouds ? 1.0f : 0.0f);

		const float SkyTintStrength = bIsCurrentlyNight
			? TwoDCloudsNightTimeSkyTintStrength
			: TwoDCloudsDayTimeSkyTintStrength;
		const FLinearColor TwoDCloudSettings(
			TwoDCloudsTiling,
			TwoDCloudsPanningSpeed,
			TwoDCloudsBrightness,
			SkyTintStrength);
		SkySphereMID->SetVectorParameterValue(DynamicSkyParameterNames::TwoDCloudSettings, TwoDCloudSettings);
	}

	if (VolumetricCloud)
	{
		const bool bShowVolumetricCloud = CurrentCloudMode == EDynamicSkyCloudMode::VolumetricClouds;
		VolumetricCloud->SetVisibility(bShowVolumetricCloud, true);
		VolumetricCloud->SetHiddenInGame(!bShowVolumetricCloud);
		VolumetricCloud->SetLayerBottomAltitude(VolumCloudLayerBottomAltitude);
		VolumetricCloud->SetLayerHeight(VolumCloudLayerHeight);
	}

	if (VolumetricCloudMID)
	{
		VolumetricCloudMID->SetScalarParameterValue(DynamicSkyParameterNames::PanningSpeed, VolumCloudMovingSpeed);
	}

	ApplyCloudTimeToMaterial();
}

void ADynamicSky::ApplyCloudTimeToMaterial()
{
	if (!VolumetricCloudMID)
	{
		return;
	}

	VolumetricCloudMID->SetScalarParameterValue(
		DynamicSkyParameterNames::CloudSyncTime,
		CalculateCurrentCloudTime());
}

void ADynamicSky::AdvanceCloudTime(float DeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	CloudSyncUpdateElapsed += DeltaTime;
	const float EffectiveSyncInterval = FMath::Max(CloudSyncInterval, 0.05f);
	if (CloudSyncUpdateElapsed < EffectiveSyncInterval)
	{
		return;
	}

	CloudAccumulatedTime = CalculateCurrentCloudTime();
	ResetCloudSyncBase();
	ForceNetUpdate();
}

void ADynamicSky::ResetCloudSyncBase()
{
	CloudSyncBaseTime = CloudAccumulatedTime;
	CloudSyncUpdateElapsed = 0.0f;

	if (const UWorld* World = GetWorld())
	{
		CloudSyncBaseLocalTime = World->GetTimeSeconds();
	}
	else
	{
		CloudSyncBaseLocalTime = 0.0f;
	}
}

float ADynamicSky::CalculateCurrentCloudTime() const
{
	if (const UWorld* World = GetWorld())
	{
		const float ElapsedLocalTime = FMath::Max(World->GetTimeSeconds() - CloudSyncBaseLocalTime, 0.0f);
		return CloudSyncBaseTime + ElapsedLocalTime;
	}

	return CloudAccumulatedTime;
}

void ADynamicSky::UpdateDayNightState(bool bBroadcastEvents)
{
	const bool bNewNightState = IsNightTime(TimeOfDay);
	const bool bStateChanged = bIsCurrentlyNight != bNewNightState;

	if (bStateChanged)
	{
		bIsCurrentlyNight = bNewNightState;
	}

	if (bStateChanged && bBroadcastEvents)
	{
		if (bIsCurrentlyNight)
		{
			OnNightStarted.Broadcast();
			BP_OnNightStarted();
		}
		else
		{
			OnDayStarted.Broadcast();
			BP_OnDayStarted();
		}
	}

	if (HasAuthority() && HasActorBegunPlay())
	{
		UpdateNightSystemActivation(bIsCurrentlyNight);

		if (bStateChanged)
		{
			ApplyEnemyFormsToRegisteredEnemies();
		}
	}
}

void ADynamicSky::UpdateNightSystemActivation(bool bShouldActivate)
{
	if (!HasAuthority() || bNightSystemActivated == bShouldActivate)
	{
		return;
	}

	AMoonlightInfectionSystem* InfectionSystem = ResolveMoonlightInfectionSystem();
	if (!IsValid(InfectionSystem))
	{
		return;
	}

	if (bShouldActivate)
	{
		InfectionSystem->ActivateInfectionCheck();
	}
	else
	{
		InfectionSystem->DeactivateInfectionCheck();
	}

	bNightSystemActivated = bShouldActivate;
}

AMoonlightInfectionSystem* ADynamicSky::ResolveMoonlightInfectionSystem()
{
	if (IsValid(MoonlightInfectionSystem))
	{
		return MoonlightInfectionSystem;
	}

	MoonlightInfectionSystem = Cast<AMoonlightInfectionSystem>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AMoonlightInfectionSystem::StaticClass()));

	return MoonlightInfectionSystem;
}

void ADynamicSky::RegisterExistingEnemies()
{
	if (!HasAuthority() || !GetWorld())
	{
		return;
	}

	for (TActorIterator<AEnemyAIBase> EnemyIterator(GetWorld()); EnemyIterator; ++EnemyIterator)
	{
		RegisterEnemy(*EnemyIterator);
	}
}

void ADynamicSky::HandleActorSpawned(AActor* SpawnedActor)
{
	if (!HasAuthority() || !IsValid(Cast<AEnemyAIBase>(SpawnedActor)))
	{
		return;
	}

	RegisterEnemy(SpawnedActor);
}

void ADynamicSky::ScheduleEnemyFormRefresh()
{
	if (!HasAuthority() || bEnemyFormRefreshScheduled || !GetWorld())
	{
		return;
	}

	bEnemyFormRefreshScheduled = true;
	GetWorldTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateUObject(this, &ADynamicSky::HandleScheduledEnemyFormRefresh));
}

void ADynamicSky::HandleScheduledEnemyFormRefresh()
{
	bEnemyFormRefreshScheduled = false;
	ApplyEnemyFormsToRegisteredEnemies();
}

void ADynamicSky::ApplyEnemyFormsToRegisteredEnemies()
{
	if (!HasAuthority())
	{
		return;
	}

	for (int32 EnemyIndex = RegisteredEnemies.Num() - 1; EnemyIndex >= 0; --EnemyIndex)
	{
		AEnemyAIBase* EnemyActor = Cast<AEnemyAIBase>(RegisteredEnemies[EnemyIndex]);
		if (!IsValid(EnemyActor))
		{
			RegisteredEnemies.RemoveAtSwap(EnemyIndex);
			continue;
		}

		ApplyEnemyForm(EnemyActor);
	}
}

void ADynamicSky::ApplyEnemyForm(AEnemyAIBase* EnemyActor) const
{
	if (!HasAuthority() || !IsValid(EnemyActor))
	{
		return;
	}

	EnemyActor->ServerChangeForm(bIsCurrentlyNight ? EEnemyForm::Wolf : EEnemyForm::Human);
}

bool ADynamicSky::IsNightTime(float TestTimeOfDay) const
{
	const float NormalizedTime = DynamicSkyParameterNames::NormalizeTimeOfDay(TestTimeOfDay);
	const float NormalizedDayStart = DynamicSkyParameterNames::NormalizeTimeOfDay(
		DawnTime - DynamicSkyParameterNames::Ver3DayNightTransitionOffsetHours);
	const float NormalizedDayEnd = DynamicSkyParameterNames::NormalizeTimeOfDay(
		DuskTime + DynamicSkyParameterNames::Ver3DayNightTransitionOffsetHours);

	if (FMath::IsNearlyEqual(NormalizedDayStart, NormalizedDayEnd))
	{
		return false;
	}

	const bool bIsDaytime = NormalizedDayStart < NormalizedDayEnd
		? NormalizedTime > NormalizedDayStart && NormalizedTime < NormalizedDayEnd
		: NormalizedTime > NormalizedDayStart || NormalizedTime < NormalizedDayEnd;

	return !bIsDaytime;
}

float ADynamicSky::CalculateSunPitch(float TestTimeOfDay) const
{
	return DynamicSkyParameterNames::MapTimeOfDayUnclamped(
		TestTimeOfDay,
		DawnTime,
		DuskTime,
		SunPitchAtDawn,
		SunPitchAtDusk);
}

float ADynamicSky::CalculateMoonBillboardRotationStatic(float TestMoonPitch) const
{
	bool bMoonPassedZenith = TestMoonPitch <= MoonBillboardFlipThresholdPitch;
	if (bInvertMoonBillboardFlip)
	{
		bMoonPassedZenith = !bMoonPassedZenith;
	}

	return bMoonPassedZenith ? MoonBillboardRotationAfterZenith : MoonBillboardRotationBeforeZenith;
}

void ADynamicSky::EnsureSkyManagerTag()
{
	Tags.AddUnique(FName(TEXT("SkyManager")));
}
