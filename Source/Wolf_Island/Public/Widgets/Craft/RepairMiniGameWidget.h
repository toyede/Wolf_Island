// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemDataStruct.h"
#include "RepairMiniGameWidget.generated.h"

class UImage;
class UTextBlock;
class UButton;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRepairMiniGameFinished, bool, bSuccess);

UCLASS()
class WOLF_ISLAND_API URepairMiniGameWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Repair|MiniGame")
    FOnRepairMiniGameFinished OnMiniGameFinished;

    // 블루프린트 위젯 호환성을 위해 이름 유지 (실제로는 '수축하는 외곽선 원' 역할)
    UPROPERTY(VisibleAnywhere, meta=(BindWidgetOptional))
    UImage* HammerImage;

    // 블루프린트 위젯 호환성을 위해 이름 유지 (실제로는 '고정된 타겟 원' 역할)
    UPROPERTY(VisibleAnywhere, meta=(BindWidgetOptional))
    UImage* ZoneImage;

    // (선택 사항) 배경 원 이미지
    UPROPERTY(VisibleAnywhere, meta=(BindWidgetOptional))
    UImage* TrackImage;

    UPROPERTY(VisibleAnywhere, meta=(BindWidgetOptional))
    UButton* CheckButton;

    UPROPERTY(VisibleAnywhere, meta=(BindWidgetOptional))
    UButton* CancelButton;

    UPROPERTY(VisibleAnywhere, meta=(BindWidgetOptional))
    UTextBlock* RemainingSuccessText;

    UFUNCTION(BlueprintCallable, Category = "Repair|MiniGame")
    void StartMiniGame(const FRepairRecipeData& RepairData);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    UFUNCTION()
    void OnCheckButtonClicked();

    UFUNCTION()
    void OnCancelButtonClicked();

    void StopMiniGame(bool bCompleted);
    void UpdateRemainingText();
    
    // 이동 대신 스케일을 줄이는 함수로 변경
    void UpdateCircleScale(float DeltaTime);
    
    void SpawnCircle();
    void HideCircleAndScheduleRespawn();
    void RandomizeShrinkSpeed();
    
    bool IsCircleInZone() const;
    void FlashZoneFail();
    void FlashZoneSuccess();
    void ResetZoneColor();

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    int32 TargetSuccessCount = 5;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float CircleStartScale = 2.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float SuccessTargetScaleMin = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float SuccessTargetScaleMax = 1.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float SuccessTolerance = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float ShrinkSpeedMin = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float ShrinkSpeedMax = 1.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float SpeedChangeIntervalMin = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float SpeedChangeIntervalMax = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float RespawnDelayMin = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float RespawnDelayMax = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float DifficultySpeedMultiplierMax = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float DifficultyRespawnMultiplierMin = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    FLinearColor FailFlashColor = FLinearColor(0.92f, 0.58f, 0.58f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float FailFlashDuration = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    FLinearColor SuccessFlashColor = FLinearColor(0.64f, 0.92f, 0.69f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float SuccessFlashDuration = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|Sound")
    USoundBase* SuccessSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|Sound")
    USoundBase* FailSound;

private:
    bool bMiniGameActive = false;
    bool bCircleActive = false;
    bool bMissedCurrentCircle = false;
    
    float CurrentCircleScale = 1.0f;
    float CurrentShrinkSpeed = 1.0f;
    float CurrentTargetScale = 1.0f;
    
    int32 CurrentSuccesses = 0;
    FRepairRecipeData CurrentRepairData;
    
    FTimerHandle SpeedChangeTimer;
    FTimerHandle FailFlashTimer;
    FTimerHandle SuccessFlashTimer;
    FTimerHandle RespawnTimer;
    FLinearColor ZoneOriginalColor = FLinearColor::White;
};
