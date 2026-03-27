// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsWidget.generated.h"

/**
 * 
 */
UCLASS()
class WOLF_ISLAND_API USettingsWidget : public UUserWidget
{
	GENERATED_BODY()
public:

    // ───────────────────────────────
    // 초기화
    // ───────────────────────────────

    // 위젯이 열릴 때 현재 설정값을 Pending 변수에 로드
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void InitializeSettings();


    // ───────────────────────────────
    // 목록 반환 (콤보박스 옵션용)
    // ───────────────────────────────

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


    // ───────────────────────────────
    // 현재값 반환 (UI 초기 선택값용)
    // ───────────────────────────────

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


    // ───────────────────────────────
    // 값 변경 (UI 조작 시 호출)
    // ───────────────────────────────

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


    // ───────────────────────────────
    // 적용 및 저장
    // ───────────────────────────────

    // Apply 버튼 누를 때 호출 ? 모든 Pending 값을 실제로 적용하고 저장
    UFUNCTION(BlueprintCallable, Category = "Settings")
    void ApplySettings();


private:

    // 해상도 목록 (FIntPoint 배열)
    TArray<FIntPoint> ResolutionList;

    // 프레임 제한 목록 (실제 fps 값)
    TArray<float> FrameRateLimitList;

    // ── Pending 변수 (Apply 전까지 임시 보관) ──
    int32 PendingResolutionIndex = 2;   // 기본 1920x1080
    int32 PendingDisplayModeIndex = 0;   // 기본 풀스크린
    bool  bPendingVSync = false;
    int32 PendingFrameRateLimitIndex = 1;   // 기본 60fps
    int32 PendingTextureQuality = 3;   // 기본 최고
    int32 PendingShadowQuality = 3;
    int32 PendingEffectQuality = 3;
    int32 PendingAntiAliasingIndex = 2;   // 기본 TAA

    float PendingMasterVolume = 1.0f;
    float PendingBGMVolume = 1.0f;
    float PendingSFXVolume = 1.0f;
};
