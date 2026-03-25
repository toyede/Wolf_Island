// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Setting/SettingsWidget.h"
#include "GameFramework/GameUserSettings.h"


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
    if (!Settings) return;

    FIntPoint CurrentRes = Settings->GetScreenResolution();
    PendingResolutionIndex = 2;
    for (int32 i = 0; i < ResolutionList.Num(); i++)
    {
        if (ResolutionList[i] == CurrentRes)
        {
            PendingResolutionIndex = i;
            break;
        }
    }

    EWindowMode::Type CurrentMode = Settings->GetFullscreenMode();
    if (CurrentMode == EWindowMode::WindowedFullscreen) PendingDisplayModeIndex = 0;
    else if (CurrentMode == EWindowMode::Fullscreen)    PendingDisplayModeIndex = 0;
    else if (CurrentMode == EWindowMode::Windowed)      PendingDisplayModeIndex = 1;

    bPendingVSync = Settings->IsVSyncEnabled();

    float CurrentFPS = Settings->GetFrameRateLimit();
    PendingFrameRateLimitIndex = 0;
    for (int32 i = 0; i < FrameRateLimitList.Num(); i++)
    {
        if (FMath::IsNearlyEqual(FrameRateLimitList[i], CurrentFPS, 1.0f))
        {
            PendingFrameRateLimitIndex = i;
            break;
        }
    }

    PendingTextureQuality = Settings->GetTextureQuality();
    PendingShadowQuality = Settings->GetShadowQuality();
    PendingEffectQuality = Settings->GetVisualEffectQuality();
    PendingAntiAliasingIndex = Settings->GetAntiAliasingQuality();
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
bool  USettingsWidget::GetCurrentVSyncEnabled() { return bPendingVSync; }
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
        PendingResolutionIndex = Index;
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
        PendingFrameRateLimitIndex = Index;
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
    if (!Settings) return;

    if (ResolutionList.IsValidIndex(PendingResolutionIndex))
        Settings->SetScreenResolution(ResolutionList[PendingResolutionIndex]);

    switch (PendingDisplayModeIndex)
    {
    case 0: Settings->SetFullscreenMode(EWindowMode::WindowedFullscreen); break;
    case 1: Settings->SetFullscreenMode(EWindowMode::Windowed);           break;
    }

    Settings->SetVSyncEnabled(bPendingVSync);

    if (FrameRateLimitList.IsValidIndex(PendingFrameRateLimitIndex))
        Settings->SetFrameRateLimit(FrameRateLimitList[PendingFrameRateLimitIndex]);

    Settings->SetTextureQuality(PendingTextureQuality);
    Settings->SetShadowQuality(PendingShadowQuality);
    Settings->SetVisualEffectQuality(PendingEffectQuality);
    Settings->SetAntiAliasingQuality(PendingAntiAliasingIndex);

    Settings->ApplySettings(false);
    Settings->SaveSettings();
}