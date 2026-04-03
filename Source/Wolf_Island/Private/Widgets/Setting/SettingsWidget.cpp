// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/Setting/SettingsWidget.h"

#include "Character/MainPlayer.h"
#include "Character/MainPlayerController.h"
#include "EnhancedInputDeveloperSettings.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/Pawn.h"
#include "Games/WolfGameUserSettings.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

#define LOCTEXT_NAMESPACE "SettingsWidget"

USettingsWidget::USettingsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MainPlayerInputMappingContextFinder(
		TEXT("/Game/JSY/Characters/Player/Input/IMC_MainPlayer.IMC_MainPlayer"));
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> UIInputMappingContextFinder(
		TEXT("/Game/JSY/Characters/Player/Input/UI_Input/IMC_UI.IMC_UI"));

	if (MainPlayerInputMappingContextFinder.Succeeded())
	{
		FallbackInputMappingContexts.AddUnique(MainPlayerInputMappingContextFinder.Object);
	}

	if (UIInputMappingContextFinder.Succeeded())
	{
		FallbackInputMappingContexts.AddUnique(UIInputMappingContextFinder.Object);
	}
}

void USettingsWidget::InitializeSettings()
{
	ResolutionList.Empty();
	ResolutionList.Add(FIntPoint(1280, 720));
	ResolutionList.Add(FIntPoint(1600, 900));
	ResolutionList.Add(FIntPoint(1920, 1080));
	ResolutionList.Add(FIntPoint(2560, 1440));
	ResolutionList.Add(FIntPoint(3840, 2160));

	FrameRateLimitList.Empty();
	FrameRateLimitList.Add(0.0f);
	FrameRateLimitList.Add(30.0f);
	FrameRateLimitList.Add(60.0f);
	FrameRateLimitList.Add(120.0f);
	FrameRateLimitList.Add(144.0f);

	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings)
	{
		RefreshInputKeybinds();
		return;
	}

	const FIntPoint CurrentRes = Settings->GetScreenResolution();
	PendingResolutionIndex = 2;
	for (int32 Index = 0; Index < ResolutionList.Num(); ++Index)
	{
		if (ResolutionList[Index] == CurrentRes)
		{
			PendingResolutionIndex = Index;
			break;
		}
	}

	const EWindowMode::Type CurrentMode = Settings->GetFullscreenMode();
	if (CurrentMode == EWindowMode::WindowedFullscreen)
	{
		PendingDisplayModeIndex = 0;
	}
	else if (CurrentMode == EWindowMode::Fullscreen)
	{
		PendingDisplayModeIndex = 0;
	}
	else if (CurrentMode == EWindowMode::Windowed)
	{
		PendingDisplayModeIndex = 1;
	}

	bPendingVSync = Settings->IsVSyncEnabled();

	const float CurrentFPS = Settings->GetFrameRateLimit();
	PendingFrameRateLimitIndex = 0;
	for (int32 Index = 0; Index < FrameRateLimitList.Num(); ++Index)
	{
		if (FMath::IsNearlyEqual(FrameRateLimitList[Index], CurrentFPS, 1.0f))
		{
			PendingFrameRateLimitIndex = Index;
			break;
		}
	}

	PendingTextureQuality = Settings->GetTextureQuality();
	PendingShadowQuality = Settings->GetShadowQuality();
	PendingEffectQuality = Settings->GetVisualEffectQuality();
	PendingAntiAliasingIndex = Settings->GetAntiAliasingQuality();

	if (UWolfGameUserSettings* WolfSettings = UWolfGameUserSettings::GetWolfGameUserSettings())
	{
		PendingMasterVolume = WolfSettings->MasterVolume;
		PendingBGMVolume = WolfSettings->BGMVolume;
		PendingSFXVolume = WolfSettings->SFXVolume;
	}

	RefreshInputKeybinds();
}

TArray<FString> USettingsWidget::GetResolutionOptions()
{
	return {
		TEXT("1280 x 720"),
		TEXT("1600 x 900"),
		TEXT("1920 x 1080"),
		TEXT("2560 x 1440"),
		TEXT("3840 x 2160")
	};
}

TArray<FString> USettingsWidget::GetDisplayModeOptions()
{
	return {
		TEXT("Fullscreen"),
		TEXT("Windowed")
	};
}

TArray<FString> USettingsWidget::GetFrameRateLimitOptions()
{
	return {
		TEXT("Unlimited"),
		TEXT("30"),
		TEXT("60"),
		TEXT("120"),
		TEXT("144")
	};
}

TArray<FString> USettingsWidget::GetTextureQualityOptions()
{
	return {
		TEXT("Low"),
		TEXT("Medium"),
		TEXT("High"),
		TEXT("Ultra")
	};
}

TArray<FString> USettingsWidget::GetShadowQualityOptions()
{
	return {
		TEXT("Low"),
		TEXT("Medium"),
		TEXT("High"),
		TEXT("Ultra")
	};
}

TArray<FString> USettingsWidget::GetEffectQualityOptions()
{
	return {
		TEXT("Low"),
		TEXT("Medium"),
		TEXT("High"),
		TEXT("Ultra")
	};
}

TArray<FString> USettingsWidget::GetAntiAliasingOptions()
{
	return {
		TEXT("None"),
		TEXT("FXAA"),
		TEXT("TAA"),
		TEXT("TSR")
	};
}

int32 USettingsWidget::GetCurrentResolutionIndex() { return PendingResolutionIndex; }
int32 USettingsWidget::GetCurrentDisplayModeIndex() { return PendingDisplayModeIndex; }
bool USettingsWidget::GetCurrentVSyncEnabled() { return bPendingVSync; }
int32 USettingsWidget::GetCurrentFrameRateLimitIndex() { return PendingFrameRateLimitIndex; }
int32 USettingsWidget::GetCurrentTextureQualityIndex() { return PendingTextureQuality; }
int32 USettingsWidget::GetCurrentShadowQualityIndex() { return PendingShadowQuality; }
int32 USettingsWidget::GetCurrentEffectQualityIndex() { return PendingEffectQuality; }
int32 USettingsWidget::GetCurrentAntiAliasingIndex() { return PendingAntiAliasingIndex; }
float USettingsWidget::GetMasterVolume() { return PendingMasterVolume; }
float USettingsWidget::GetBGMVolume() { return PendingBGMVolume; }
float USettingsWidget::GetSFXVolume() { return PendingSFXVolume; }

void USettingsWidget::SetResolution(int32 Index)
{
	if (ResolutionList.IsValidIndex(Index))
	{
		PendingResolutionIndex = Index;
	}
}

void USettingsWidget::SetDisplayMode(int32 Index)
{
	PendingDisplayModeIndex = Index;
}

void USettingsWidget::SetVSyncEnabled(bool bEnabled)
{
	bPendingVSync = bEnabled;
}

void USettingsWidget::SetFrameRateLimit(int32 Index)
{
	if (FrameRateLimitList.IsValidIndex(Index))
	{
		PendingFrameRateLimitIndex = Index;
	}
}

void USettingsWidget::SetTextureQuality(int32 Index)
{
	PendingTextureQuality = FMath::Clamp(Index, 0, 3);
}

void USettingsWidget::SetShadowQuality(int32 Index)
{
	PendingShadowQuality = FMath::Clamp(Index, 0, 3);
}

void USettingsWidget::SetEffectQuality(int32 Index)
{
	PendingEffectQuality = FMath::Clamp(Index, 0, 3);
}

void USettingsWidget::SetAntiAliasing(int32 Index)
{
	PendingAntiAliasingIndex = FMath::Clamp(Index, 0, 3);
}

void USettingsWidget::SetMasterVolume(float Value)
{
	PendingMasterVolume = FMath::Clamp(Value, 0.0f, 1.0f);
}

void USettingsWidget::SetBGMVolume(float Value)
{
	PendingBGMVolume = FMath::Clamp(Value, 0.0f, 1.0f);
}

void USettingsWidget::SetSFXVolume(float Value)
{
	PendingSFXVolume = FMath::Clamp(Value, 0.0f, 1.0f);
}

void USettingsWidget::ApplySettings()
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings)
	{
		return;
	}

	if (ResolutionList.IsValidIndex(PendingResolutionIndex))
	{
		Settings->SetScreenResolution(ResolutionList[PendingResolutionIndex]);
	}

	switch (PendingDisplayModeIndex)
	{
	case 0:
		Settings->SetFullscreenMode(EWindowMode::WindowedFullscreen);
		break;
	case 1:
		Settings->SetFullscreenMode(EWindowMode::Windowed);
		break;
	default:
		break;
	}

	Settings->SetVSyncEnabled(bPendingVSync);

	if (FrameRateLimitList.IsValidIndex(PendingFrameRateLimitIndex))
	{
		Settings->SetFrameRateLimit(FrameRateLimitList[PendingFrameRateLimitIndex]);
	}

	Settings->SetTextureQuality(PendingTextureQuality);
	Settings->SetShadowQuality(PendingShadowQuality);
	Settings->SetVisualEffectQuality(PendingEffectQuality);
	Settings->SetAntiAliasingQuality(PendingAntiAliasingIndex);

	Settings->ApplySettings(false);
	Settings->SaveSettings();

	if (UWolfGameUserSettings* WolfSettings = UWolfGameUserSettings::GetWolfGameUserSettings())
	{
		WolfSettings->MasterVolume = PendingMasterVolume;
		WolfSettings->BGMVolume = PendingBGMVolume;
		WolfSettings->SFXVolume = PendingSFXVolume;
		WolfSettings->SaveSettings();
	}

	if (GameSoundMix)
	{
		UGameplayStatics::SetBaseSoundMix(GetWorld(), GameSoundMix);
		if (MasterSoundClass)
			UGameplayStatics::SetSoundMixClassOverride(GetWorld(), GameSoundMix, MasterSoundClass, PendingMasterVolume, 1.0f, 0.0f);
		if (BGMSoundClass)
			UGameplayStatics::SetSoundMixClassOverride(GetWorld(), GameSoundMix, BGMSoundClass, PendingBGMVolume, 1.0f, 0.0f);
		if (SFXSoundClass)
			UGameplayStatics::SetSoundMixClassOverride(GetWorld(), GameSoundMix, SFXSoundClass, PendingSFXVolume, 1.0f, 0.0f);
		UGameplayStatics::PushSoundMixModifier(GetWorld(), GameSoundMix);
	}
}

void USettingsWidget::RefreshInputKeybinds()
{
	InputKeybindEntries.Reset();
	SetLastInputKeybindMessage(FText::GetEmpty());

	UEnhancedPlayerMappableKeyProfile* KeyProfile = GetCurrentKeyProfile(true);
	if (!KeyProfile)
	{
		SetLastInputKeybindMessage(LOCTEXT("InputSettingsUnavailable", "입력 설정을 초기화하지 못했습니다."));
		OnInputKeybindsChanged();
		return;
	}

	BuildInputKeybindEntries(KeyProfile);
	if (InputKeybindEntries.IsEmpty())
	{
		SetLastInputKeybindMessage(LOCTEXT("NoPlayerMappableInputs", "플레이어 매핑 가능한 입력을 찾지 못했습니다. Input Action / IMC 의 Player Mappable 설정을 확인하세요."));
	}

	OnInputKeybindsChanged();
}

TArray<FSettingsInputKeybindEntry> USettingsWidget::GetInputKeybinds() const
{
	return InputKeybindEntries;
}

bool USettingsWidget::HasInputKeybinds() const
{
	return InputKeybindEntries.Num() > 0;
}

FText USettingsWidget::GetLastInputKeybindMessage() const
{
	return LastInputKeybindMessage;
}

bool USettingsWidget::ApplyInputKeybind(FName MappingName, EPlayerMappableKeySlot KeySlot, FKey NewKey)
{
	if (MappingName.IsNone())
	{
		SetLastInputKeybindMessage(LOCTEXT("InvalidMappingName", "잘못된 입력 매핑입니다."));
		OnInputKeybindsChanged();
		return false;
	}

	if (!NewKey.IsValid())
	{
		SetLastInputKeybindMessage(LOCTEXT("InvalidNewKey", "유효한 키를 선택해 주세요."));
		OnInputKeybindsChanged();
		return false;
	}

	if (IsAxisKey(NewKey))
	{
		SetLastInputKeybindMessage(LOCTEXT("AxisKeyNotSupported", "축 입력은 현재 설정 UI에서 변경하지 않습니다."));
		OnInputKeybindsChanged();
		return false;
	}

	UEnhancedInputUserSettings* UserSettings = GetInputUserSettings(true);
	UEnhancedPlayerMappableKeyProfile* KeyProfile = UserSettings ? UserSettings->GetCurrentKeyProfile() : nullptr;
	if (!UserSettings || !KeyProfile)
	{
		SetLastInputKeybindMessage(LOCTEXT("MapPlayerKeyNoUserSettings", "입력 설정을 불러오지 못했습니다."));
		OnInputKeybindsChanged();
		return false;
	}

	FMapPlayerKeyArgs Args;
	Args.MappingName = MappingName;
	Args.Slot = KeySlot;
	Args.NewKey = NewKey;
	Args.bCreateMatchingSlotIfNeeded = true;

	FPlayerKeyMapping ExistingMapping;
	KeyProfile->K2_FindKeyMapping(ExistingMapping, Args);

	FText LockReason;
	if (ExistingMapping.IsValid() && !ShouldAllowRebind(ExistingMapping, LockReason))
	{
		SetLastInputKeybindMessage(LockReason);
		OnInputKeybindsChanged();
		return false;
	}

	if (ExistingMapping.IsValid() && ExistingMapping.GetCurrentKey() == NewKey)
	{
		SetLastInputKeybindMessage(LOCTEXT("SameKeySelected", "이미 같은 키로 설정되어 있습니다."));
		OnInputKeybindsChanged();
		return true;
	}

	FGameplayTagContainer FailureReason;
	UserSettings->MapPlayerKey(Args, FailureReason);
	if (FailureReason.IsValid())
	{
		RefreshInputKeybinds();
		SetLastInputKeybindMessage(LOCTEXT("MapPlayerKeyFailed", "입력 변경에 실패했습니다. 이미 사용 중인 키이거나 허용되지 않는 조합일 수 있습니다."));
		OnInputKeybindsChanged();
		return false;
	}

	UserSettings->ApplySettings();
	UserSettings->SaveSettings();

	RefreshInputKeybinds();
	SetLastInputKeybindMessage(FText::Format(
		LOCTEXT("MapPlayerKeySuccess", "{0} 입력이 {1}(으)로 변경되었습니다."),
		MakeFallbackDisplayName(MappingName),
		NewKey.GetDisplayName()));
	OnInputKeybindsChanged();
	return true;
}

bool USettingsWidget::ApplyInputKeybindFromChord(FName MappingName, EPlayerMappableKeySlot KeySlot, const FInputChord& NewChord)
{
	return ApplyInputKeybind(MappingName, KeySlot, NewChord.Key);
}

bool USettingsWidget::ResetInputKeybind(FName MappingName)
{
	if (MappingName.IsNone())
	{
		SetLastInputKeybindMessage(LOCTEXT("ResetInvalidMappingName", "초기화할 입력 매핑이 없습니다."));
		OnInputKeybindsChanged();
		return false;
	}

	UEnhancedInputUserSettings* UserSettings = GetInputUserSettings(true);
	if (!UserSettings)
	{
		SetLastInputKeybindMessage(LOCTEXT("ResetNoUserSettings", "입력 설정을 불러오지 못했습니다."));
		OnInputKeybindsChanged();
		return false;
	}

	FMapPlayerKeyArgs Args;
	Args.MappingName = MappingName;

	FGameplayTagContainer FailureReason;
	UserSettings->ResetAllPlayerKeysInRow(Args, FailureReason);
	if (FailureReason.IsValid())
	{
		RefreshInputKeybinds();
		SetLastInputKeybindMessage(LOCTEXT("ResetInputKeyFailed", "입력 초기화에 실패했습니다."));
		OnInputKeybindsChanged();
		return false;
	}

	UserSettings->ApplySettings();
	UserSettings->SaveSettings();

	RefreshInputKeybinds();
	SetLastInputKeybindMessage(FText::Format(
		LOCTEXT("ResetInputKeySuccess", "{0} 입력을 기본값으로 되돌렸습니다."),
		MakeFallbackDisplayName(MappingName)));
	OnInputKeybindsChanged();
	return true;
}

void USettingsWidget::ResetAllInputKeybinds()
{
	UEnhancedInputUserSettings* UserSettings = GetInputUserSettings(true);
	UEnhancedPlayerMappableKeyProfile* KeyProfile = UserSettings ? UserSettings->GetCurrentKeyProfile() : nullptr;
	if (!UserSettings || !KeyProfile)
	{
		SetLastInputKeybindMessage(LOCTEXT("ResetAllNoUserSettings", "입력 설정을 불러오지 못했습니다."));
		OnInputKeybindsChanged();
		return;
	}

	KeyProfile->ResetToDefault();
	UserSettings->ApplySettings();
	UserSettings->SaveSettings();

	RefreshInputKeybinds();
	SetLastInputKeybindMessage(LOCTEXT("ResetAllInputKeysSuccess", "모든 입력을 기본값으로 되돌렸습니다."));
	OnInputKeybindsChanged();
}

UEnhancedInputLocalPlayerSubsystem* USettingsWidget::GetEnhancedInputSubsystem() const
{
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		return LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	}

	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (ULocalPlayer* FirstGamePlayer = GameInstance->GetFirstGamePlayer())
		{
			return FirstGamePlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		}
	}

	if (const UWorld* World = GetWorld())
	{
		if (ULocalPlayer* FirstLocalPlayer = World->GetFirstLocalPlayerFromController())
		{
			return FirstLocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		}
	}

	return nullptr;
}

UEnhancedInputUserSettings* USettingsWidget::GetInputUserSettings(bool bTryInitialize)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem();
	if (!Subsystem)
	{
		return nullptr;
	}

	if (bTryInitialize)
	{
		// The project may not have toggled this setting in the editor yet, so bootstrap it here as a fallback.
		UEnhancedInputDeveloperSettings* DeveloperSettings = GetMutableDefault<UEnhancedInputDeveloperSettings>();
		if (DeveloperSettings && !DeveloperSettings->bEnableUserSettings)
		{
			DeveloperSettings->bEnableUserSettings = true;
			DeveloperSettings->SaveConfig();
		}

		Subsystem->InitalizeUserSettings();
	}

	UEnhancedInputUserSettings* UserSettings = Subsystem->GetUserSettings();
	if (UserSettings && bTryInitialize)
	{
		RegisterRelevantInputMappingContexts(UserSettings);
	}

	return UserSettings;
}

UEnhancedPlayerMappableKeyProfile* USettingsWidget::GetCurrentKeyProfile(bool bTryInitialize)
{
	UEnhancedInputUserSettings* UserSettings = GetInputUserSettings(bTryInitialize);
	return UserSettings ? UserSettings->GetCurrentKeyProfile() : nullptr;
}

TArray<const UInputMappingContext*> USettingsWidget::CollectRelevantInputMappingContexts() const
{
	TArray<const UInputMappingContext*> Contexts;

	if (const AMainPlayerController* MainPlayerController = Cast<AMainPlayerController>(GetOwningPlayer()))
	{
		if (MainPlayerController->InputMappingContext)
		{
			Contexts.AddUnique(MainPlayerController->InputMappingContext);
		}

		if (const AMainPlayer* MainPlayer = Cast<AMainPlayer>(MainPlayerController->GetPawn()))
		{
			if (MainPlayer->InputMappingContext)
			{
				Contexts.AddUnique(MainPlayer->InputMappingContext);
			}
		}
	}
	else if (const AMainPlayer* MainPlayer = Cast<AMainPlayer>(GetOwningPlayerPawn()))
	{
		if (MainPlayer->InputMappingContext)
		{
			Contexts.AddUnique(MainPlayer->InputMappingContext);
		}
	}

	for (const UInputMappingContext* MappingContext : FallbackInputMappingContexts)
	{
		if (MappingContext)
		{
			Contexts.AddUnique(MappingContext);
		}
	}

	return Contexts;
}

void USettingsWidget::RegisterRelevantInputMappingContexts(UEnhancedInputUserSettings* UserSettings) const
{
	if (!UserSettings)
	{
		return;
	}

	for (const UInputMappingContext* MappingContext : CollectRelevantInputMappingContexts())
	{
		if (MappingContext)
		{
			UserSettings->RegisterInputMappingContext(MappingContext);
		}
	}
}

void USettingsWidget::BuildInputKeybindEntries(const UEnhancedPlayerMappableKeyProfile* KeyProfile)
{
	InputKeybindEntries.Reset();
	if (!KeyProfile)
	{
		return;
	}

	for (const TPair<FName, FKeyMappingRow>& Pair : KeyProfile->GetPlayerMappingRows())
	{
		for (const FPlayerKeyMapping& Mapping : Pair.Value.Mappings)
		{
			const FKey CurrentKey = Mapping.GetCurrentKey();
			const FKey DefaultKey = Mapping.GetDefaultKey();

			if (!CurrentKey.IsValid() && !DefaultKey.IsValid())
			{
				continue;
			}

			const bool bIsGamepadOnly =
				(CurrentKey.IsValid() ? CurrentKey.IsGamepadKey() : true) &&
				(DefaultKey.IsValid() ? DefaultKey.IsGamepadKey() : true);
			if (bIsGamepadOnly)
			{
				continue;
			}

			FText LockReason;
			const bool bCanRebind = ShouldAllowRebind(Mapping, LockReason);

			const FText DisplayName = Mapping.GetDisplayName().IsEmpty()
				? MakeFallbackDisplayName(Mapping.GetMappingName())
				: Mapping.GetDisplayName();
			const FText DisplayCategory = Mapping.GetDisplayCategory().IsEmpty()
				? LOCTEXT("DefaultInputCategory", "Input")
				: Mapping.GetDisplayCategory();

			FSettingsInputKeybindEntry Entry;
			Entry.MappingName = Mapping.GetMappingName();
			Entry.Slot = Mapping.GetSlot();
			Entry.DisplayName = DisplayName;
			Entry.DisplayCategory = DisplayCategory;
			Entry.DisplayLabel = MakeDisplayLabel(DisplayName, Mapping.GetSlot());
			Entry.CurrentKey = CurrentKey;
			Entry.CurrentKeyText = CurrentKey.IsValid()
				? CurrentKey.GetDisplayName()
				: LOCTEXT("UnboundKey", "Unbound");
			Entry.DefaultKey = DefaultKey;
			Entry.DefaultKeyText = DefaultKey.IsValid()
				? DefaultKey.GetDisplayName()
				: LOCTEXT("UnboundKey", "Unbound");
			Entry.bCanRebind = bCanRebind;
			Entry.LockReason = LockReason;

			InputKeybindEntries.Add(MoveTemp(Entry));
		}
	}

	InputKeybindEntries.Sort([](const FSettingsInputKeybindEntry& A, const FSettingsInputKeybindEntry& B)
	{
		const int32 CategoryCompare = A.DisplayCategory.ToString().Compare(B.DisplayCategory.ToString(), ESearchCase::IgnoreCase);
		if (CategoryCompare != 0)
		{
			return CategoryCompare < 0;
		}

		const int32 LabelCompare = A.DisplayLabel.ToString().Compare(B.DisplayLabel.ToString(), ESearchCase::IgnoreCase);
		if (LabelCompare != 0)
		{
			return LabelCompare < 0;
		}

		return static_cast<int32>(A.Slot) < static_cast<int32>(B.Slot);
	});
}

FText USettingsWidget::MakeFallbackDisplayName(FName MappingName) const
{
	FString DisplayString = MappingName.ToString();
	DisplayString.RemoveFromStart(TEXT("IA_"));
	DisplayString.RemoveFromStart(TEXT("InputAction_"));
	DisplayString.ReplaceInline(TEXT("_"), TEXT(" "));
	return FText::FromString(DisplayString);
}

FText USettingsWidget::MakeDisplayLabel(const FText& BaseDisplayName, EPlayerMappableKeySlot KeySlot) const
{
	if (KeySlot == EPlayerMappableKeySlot::First || KeySlot == EPlayerMappableKeySlot::Unspecified)
	{
		return BaseDisplayName;
	}

	return FText::Format(
		LOCTEXT("AlternateSlotLabel", "{0} ({1})"),
		BaseDisplayName,
		FText::AsNumber(static_cast<int32>(KeySlot) + 1));
}

bool USettingsWidget::ShouldAllowRebind(const FPlayerKeyMapping& Mapping, FText& OutReason) const
{
	const FKey CurrentKey = Mapping.GetCurrentKey();
	const FKey DefaultKey = Mapping.GetDefaultKey();

	if (IsAxisKey(CurrentKey) || IsAxisKey(DefaultKey))
	{
		OutReason = LOCTEXT("AxisBindingLocked", "축 입력은 현재 설정 UI에서 변경하지 않습니다.");
		return false;
	}

	return true;
}

bool USettingsWidget::IsAxisKey(const FKey& Key) const
{
	if (!Key.IsValid())
	{
		return false;
	}

	return Key.IsAxis1D() || Key.IsAxis2D() || Key.IsAxis3D();
}

void USettingsWidget::SetLastInputKeybindMessage(const FText& Message)
{
	LastInputKeybindMessage = Message;
}

#undef LOCTEXT_NAMESPACE
