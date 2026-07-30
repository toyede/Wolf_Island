#include "Actors/DynamicSky.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Moon/EclipseManagerComponent.h"
#include "Net/UnrealNetwork.h"

namespace DynamicSkyParameterNames
{
	static constexpr float FullDayHours = 24.0f;
	static constexpr float SecondsPerMinute = 60.0f;

	static const FName Panning(TEXT("Panning"));
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

	float GetHoursSince(float StartTime, float EndTime)
	{
		return NormalizeTimeOfDay(EndTime - StartTime);
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

	MoonDirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("MoonDirectionalLight"));
	MoonDirectionalLight->SetupAttachment(DefaultSceneRoot);
	MoonDirectionalLight->SetIntensity(1.0f);
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
	DayLengthMinutes = 24.0f;
	DaysPassed = 0;
	CurrentDay = 0;
	bTimePaused = false;
	bIsCurrentlyNight = false;
	bMidnightTriggered = false;

	bControlSunMoonRotation = true;
	bControlSunMoonVisibility = true;
	SunHorizonPitch = 0.0f;
	SunNoonPitch = -90.0f;
	SunYaw = 180.0f;
	MoonYaw = 0.0f;
	SunRotationOffset = FRotator::ZeroRotator;
	MoonRotationOffset = FRotator::ZeroRotator;

	CloudAccumulatedTime = 0.0f;
	CloudSyncBaseTime = 0.0f;
	CloudSyncBaseLocalTime = 0.0f;
	CloudSyncInterval = 2.0f;
	CurrentCloudMode = EDynamicSkyCloudMode::None;
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

void ADynamicSky::BeginPlay()
{
	Super::BeginPlay();

	EnsureSkyManagerTag();
	InitializeDynamicMaterials();
	UpdateDayNightState(false);
	RefreshSkyFromState();
	BP_OnSkyInitialized();
}

void ADynamicSky::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority())
	{
		AdvanceTime(DeltaTime);
	}
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
	Super::SaveData_Implementation(OutData);
}

void ADynamicSky::LoadData_Implementation(const FActorSaveData& InData)
{
	Super::LoadData_Implementation(InData);

	EnsureSkyManagerTag();
	InitializeDynamicMaterials();
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
	if (!EnemyActor)
	{
		return;
	}

	RegisteredEnemies.Remove(EnemyActor);
}

void ADynamicSky::SkipToMorning_Implementation()
{
	if (!HasAuthority())
	{
		return;
	}

	OnMorningSkipStarted.Broadcast();
	BP_OnMorningSkipStarted();

	if (TimeOfDay > MorningTime)
	{
		++DaysPassed;
		++CurrentDay;
	}

	TimeOfDay = DynamicSkyParameterNames::NormalizeTimeOfDay(MorningTime);
	DynamicTimeOfDay = TimeOfDay;
	bIsCurrentlyNight = false;
	bMidnightTriggered = false;

	RefreshSkyFromState();

	OnDayStarted.Broadcast();
	BP_OnDayStarted();

	ForceNetUpdate();

	OnMorningSkipFinished.Broadcast();
	BP_OnMorningSkipFinished();
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

		if (SourceMaterial)
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
	if (!EnemyActor)
	{
		return;
	}

	RegisteredEnemies.AddUnique(EnemyActor);
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
	CloudSyncBaseTime = CloudAccumulatedTime;
	if (const UWorld* World = GetWorld())
	{
		CloudSyncBaseLocalTime = World->GetTimeSeconds();
	}

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
	if (bControlSunMoonRotation)
	{
		const float SunPitch = CalculateSunPitch(TimeOfDay);
		const FRotator SunRotation = FRotator(SunPitch, SunYaw, 0.0f) + SunRotationOffset;
		const FRotator MoonRotation = FRotator(-SunPitch, MoonYaw, 0.0f) + MoonRotationOffset;

		if (SunDirectionalLight)
		{
			SunDirectionalLight->SetRelativeRotation(SunRotation);
		}

		if (MoonDirectionalLight)
		{
			MoonDirectionalLight->SetRelativeRotation(MoonRotation);
		}
	}

	if (bControlSunMoonVisibility)
	{
		const bool bShouldShowMoon = bIsCurrentlyNight;
		const bool bShouldShowSun = !bIsCurrentlyNight;

		if (SunDirectionalLight)
		{
			SunDirectionalLight->SetVisibility(bShouldShowSun, true);
			SunDirectionalLight->SetHiddenInGame(!bShouldShowSun);
		}

		if (MoonDirectionalLight)
		{
			MoonDirectionalLight->SetVisibility(bShouldShowMoon, true);
			MoonDirectionalLight->SetHiddenInGame(!bShouldShowMoon);
		}
	}
}

void ADynamicSky::ApplyCloudComponentSettings()
{
	if (VolumetricCloud)
	{
		const bool bShowVolumetricCloud = CurrentCloudMode == EDynamicSkyCloudMode::VolumetricClouds;
		VolumetricCloud->SetVisibility(bShowVolumetricCloud, true);
		VolumetricCloud->SetHiddenInGame(!bShowVolumetricCloud);
	}

	if (VolumetricCloudMID)
	{
		VolumetricCloudMID->SetScalarParameterValue(DynamicSkyParameterNames::PanningSpeed, VolumCloudMovingSpeed);
		VolumetricCloudMID->SetScalarParameterValue(DynamicSkyParameterNames::Panning, CloudAccumulatedTime);
	}
}

void ADynamicSky::UpdateDayNightState(bool bBroadcastEvents)
{
	const bool bNewNightState = IsNightTime(TimeOfDay);
	if (bIsCurrentlyNight == bNewNightState)
	{
		return;
	}

	bIsCurrentlyNight = bNewNightState;

	if (!bBroadcastEvents)
	{
		return;
	}

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

bool ADynamicSky::IsNightTime(float TestTimeOfDay) const
{
	const float NormalizedTime = DynamicSkyParameterNames::NormalizeTimeOfDay(TestTimeOfDay);
	const float NormalizedDawn = DynamicSkyParameterNames::NormalizeTimeOfDay(DawnTime);
	const float NormalizedDusk = DynamicSkyParameterNames::NormalizeTimeOfDay(DuskTime);

	if (FMath::IsNearlyEqual(NormalizedDawn, NormalizedDusk))
	{
		return false;
	}

	const bool bIsDaytime = NormalizedDawn < NormalizedDusk
		? NormalizedTime >= NormalizedDawn && NormalizedTime < NormalizedDusk
		: NormalizedTime >= NormalizedDawn || NormalizedTime < NormalizedDusk;

	return !bIsDaytime;
}

float ADynamicSky::CalculateSunPitch(float TestTimeOfDay) const
{
	const float NormalizedDawn = DynamicSkyParameterNames::NormalizeTimeOfDay(DawnTime);
	const float NormalizedDusk = DynamicSkyParameterNames::NormalizeTimeOfDay(DuskTime);
	const float DayDuration = DynamicSkyParameterNames::GetHoursSince(NormalizedDawn, NormalizedDusk);

	if (DayDuration <= KINDA_SMALL_NUMBER)
	{
		return SunHorizonPitch;
	}

	const float HoursSinceDawn = DynamicSkyParameterNames::GetHoursSince(NormalizedDawn, TestTimeOfDay);
	const float HalfDayDuration = DayDuration * 0.5f;

	if (HoursSinceDawn <= DayDuration)
	{
		if (HoursSinceDawn <= HalfDayDuration)
		{
			const float Alpha = HalfDayDuration <= KINDA_SMALL_NUMBER ? 0.0f : HoursSinceDawn / HalfDayDuration;
			return FMath::Lerp(SunHorizonPitch, SunNoonPitch, Alpha);
		}

		const float Alpha = HalfDayDuration <= KINDA_SMALL_NUMBER ? 1.0f : (HoursSinceDawn - HalfDayDuration) / HalfDayDuration;
		return FMath::Lerp(SunNoonPitch, SunHorizonPitch, Alpha);
	}

	const float NightDuration = DynamicSkyParameterNames::FullDayHours - DayDuration;
	if (NightDuration <= KINDA_SMALL_NUMBER)
	{
		return SunHorizonPitch;
	}

	const float HoursSinceDusk = DynamicSkyParameterNames::GetHoursSince(NormalizedDusk, TestTimeOfDay);
	const float HalfNightDuration = NightDuration * 0.5f;
	const float MidnightPitch = SunHorizonPitch - (SunNoonPitch - SunHorizonPitch);

	if (HoursSinceDusk <= HalfNightDuration)
	{
		const float Alpha = HalfNightDuration <= KINDA_SMALL_NUMBER ? 0.0f : HoursSinceDusk / HalfNightDuration;
		return FMath::Lerp(SunHorizonPitch, MidnightPitch, Alpha);
	}

	const float Alpha = HalfNightDuration <= KINDA_SMALL_NUMBER ? 1.0f : (HoursSinceDusk - HalfNightDuration) / HalfNightDuration;
	return FMath::Lerp(MidnightPitch, SunHorizonPitch, Alpha);
}

void ADynamicSky::EnsureSkyManagerTag()
{
	Tags.AddUnique(FName(TEXT("SkyManager")));
}
