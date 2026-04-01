// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Framework/Commands/InputChord.h"
#include "InputCoreTypes.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "SettingsWidget.generated.h"

class UInputMappingContext;

USTRUCT(BlueprintType)
struct FSettingsInputKeybindEntry
{
	GENERATED_BODY()

	FSettingsInputKeybindEntry()
		: Slot(EPlayerMappableKeySlot::First)
		, bCanRebind(false)
	{
	}

	UPROPERTY(BlueprintReadOnly, Category = "Settings|Input")
	FName MappingName = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "Settings|Input")
	EPlayerMappableKeySlot Slot;

	UPROPERTY(BlueprintReadOnly, Category = "Settings|Input")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Settings|Input")
	FText DisplayCategory;

	UPROPERTY(BlueprintReadOnly, Category = "Settings|Input")
	FText DisplayLabel;

	UPROPERTY(BlueprintReadOnly, Category = "Settings|Input")
	FKey CurrentKey;

	UPROPERTY(BlueprintReadOnly, Category = "Settings|Input")
	FText CurrentKeyText;

	UPROPERTY(BlueprintReadOnly, Category = "Settings|Input")
	FKey DefaultKey;

	UPROPERTY(BlueprintReadOnly, Category = "Settings|Input")
	FText DefaultKeyText;

	UPROPERTY(BlueprintReadOnly, Category = "Settings|Input")
	bool bCanRebind;

	UPROPERTY(BlueprintReadOnly, Category = "Settings|Input")
	FText LockReason;
};

UCLASS()
class WOLF_ISLAND_API USettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USettingsWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void InitializeSettings();

	UFUNCTION(BlueprintCallable, Category = "Settings|Options")
	TArray<FString> GetResolutionOptions();

	UFUNCTION(BlueprintCallable, Category = "Settings|Options")
	TArray<FString> GetDisplayModeOptions();

	UFUNCTION(BlueprintCallable, Category = "Settings|Options")
	TArray<FString> GetFrameRateLimitOptions();

	UFUNCTION(BlueprintCallable, Category = "Settings|Options")
	TArray<FString> GetTextureQualityOptions();

	UFUNCTION(BlueprintCallable, Category = "Settings|Options")
	TArray<FString> GetShadowQualityOptions();

	UFUNCTION(BlueprintCallable, Category = "Settings|Options")
	TArray<FString> GetEffectQualityOptions();

	UFUNCTION(BlueprintCallable, Category = "Settings|Options")
	TArray<FString> GetAntiAliasingOptions();

	UFUNCTION(BlueprintCallable, Category = "Settings|Current")
	int32 GetCurrentResolutionIndex();

	UFUNCTION(BlueprintCallable, Category = "Settings|Current")
	int32 GetCurrentDisplayModeIndex();

	UFUNCTION(BlueprintCallable, Category = "Settings|Current")
	bool GetCurrentVSyncEnabled();

	UFUNCTION(BlueprintCallable, Category = "Settings|Current")
	int32 GetCurrentFrameRateLimitIndex();

	UFUNCTION(BlueprintCallable, Category = "Settings|Current")
	int32 GetCurrentTextureQualityIndex();

	UFUNCTION(BlueprintCallable, Category = "Settings|Current")
	int32 GetCurrentShadowQualityIndex();

	UFUNCTION(BlueprintCallable, Category = "Settings|Current")
	int32 GetCurrentEffectQualityIndex();

	UFUNCTION(BlueprintCallable, Category = "Settings|Current")
	int32 GetCurrentAntiAliasingIndex();

	UFUNCTION(BlueprintCallable, Category = "Settings|Current")
	float GetMasterVolume();

	UFUNCTION(BlueprintCallable, Category = "Settings|Current")
	float GetBGMVolume();

	UFUNCTION(BlueprintCallable, Category = "Settings|Current")
	float GetSFXVolume();

	UFUNCTION(BlueprintCallable, Category = "Settings|Set")
	void SetResolution(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Settings|Set")
	void SetDisplayMode(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Settings|Set")
	void SetVSyncEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Settings|Set")
	void SetFrameRateLimit(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Settings|Set")
	void SetTextureQuality(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Settings|Set")
	void SetShadowQuality(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Settings|Set")
	void SetEffectQuality(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Settings|Set")
	void SetAntiAliasing(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Settings|Set")
	void SetMasterVolume(float Value);

	UFUNCTION(BlueprintCallable, Category = "Settings|Set")
	void SetBGMVolume(float Value);

	UFUNCTION(BlueprintCallable, Category = "Settings|Set")
	void SetSFXVolume(float Value);

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplySettings();

	UFUNCTION(BlueprintCallable, Category = "Settings|Input")
	void RefreshInputKeybinds();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Input")
	TArray<FSettingsInputKeybindEntry> GetInputKeybinds() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Input")
	bool HasInputKeybinds() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Input")
	FText GetLastInputKeybindMessage() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|Input")
	bool ApplyInputKeybind(FName MappingName, EPlayerMappableKeySlot KeySlot, FKey NewKey);

	UFUNCTION(BlueprintCallable, Category = "Settings|Input")
	bool ApplyInputKeybindFromChord(FName MappingName, EPlayerMappableKeySlot KeySlot, const FInputChord& NewChord);

	UFUNCTION(BlueprintCallable, Category = "Settings|Input")
	bool ResetInputKeybind(FName MappingName);

	UFUNCTION(BlueprintCallable, Category = "Settings|Input")
	void ResetAllInputKeybinds();

	UFUNCTION(BlueprintImplementableEvent, Category = "Settings|Input")
	void OnInputKeybindsChanged();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Input")
	TArray<TObjectPtr<UInputMappingContext>> FallbackInputMappingContexts;

private:
	TArray<FIntPoint> ResolutionList;
	TArray<float> FrameRateLimitList;

	int32 PendingResolutionIndex = 2;
	int32 PendingDisplayModeIndex = 0;
	bool bPendingVSync = false;
	int32 PendingFrameRateLimitIndex = 1;
	int32 PendingTextureQuality = 3;
	int32 PendingShadowQuality = 3;
	int32 PendingEffectQuality = 3;
	int32 PendingAntiAliasingIndex = 2;

	float PendingMasterVolume = 1.0f;
	float PendingBGMVolume = 1.0f;
	float PendingSFXVolume = 1.0f;

	TArray<FSettingsInputKeybindEntry> InputKeybindEntries;
	FText LastInputKeybindMessage;

	class UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;
	class UEnhancedInputUserSettings* GetInputUserSettings(bool bTryInitialize);
	class UEnhancedPlayerMappableKeyProfile* GetCurrentKeyProfile(bool bTryInitialize);
	TArray<const class UInputMappingContext*> CollectRelevantInputMappingContexts() const;
	void RegisterRelevantInputMappingContexts(class UEnhancedInputUserSettings* UserSettings) const;
	void BuildInputKeybindEntries(const class UEnhancedPlayerMappableKeyProfile* KeyProfile);
	FText MakeFallbackDisplayName(FName MappingName) const;
	FText MakeDisplayLabel(const FText& BaseDisplayName, EPlayerMappableKeySlot KeySlot) const;
	bool ShouldAllowRebind(const FPlayerKeyMapping& Mapping, FText& OutReason) const;
	bool IsAxisKey(const FKey& Key) const;
	void SetLastInputKeybindMessage(const FText& Message);
};
