// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Craft/RepairMiniGameWidget.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Input/Reply.h"

void URepairMiniGameWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SetIsFocusable(true);

    if (CheckButton)
    {
        CheckButton->OnClicked.AddDynamic(this, &URepairMiniGameWidget::OnCheckButtonClicked);
        CheckButton->SetIsEnabled(false);
    }

    if (CancelButton)
    {
        CancelButton->OnClicked.AddDynamic(this, &URepairMiniGameWidget::OnCancelButtonClicked);
    }
}

void URepairMiniGameWidget::StartMiniGame(const FRepairRecipeData& RepairData)
{
    if (bMiniGameActive) return;

    bMiniGameActive = true;
    CurrentRepairData = RepairData;
    CurrentSuccesses = 0;

    bCircleActive = false;
    bMissedCurrentCircle = false;
    CurrentShrinkSpeed = FMath::FRandRange(ShrinkSpeedMin, ShrinkSpeedMax);

    if (TrackImage)
    {
        ZoneOriginalColor = TrackImage->GetColorAndOpacity();
    }

    if (HammerImage)
    {
        HammerImage->SetVisibility(ESlateVisibility::Hidden);
    }

    UpdateRemainingText();

    if (CheckButton)
    {
        CheckButton->SetIsEnabled(true);
    }

    RandomizeShrinkSpeed();
    SpawnCircle();
}

void URepairMiniGameWidget::OnCheckButtonClicked()
{
    if (!bMiniGameActive || !bCircleActive) return;

    bMissedCurrentCircle = true;

    const bool bSuccess = IsCircleInZone();
    if (bSuccess)
    {
        CurrentSuccesses = FMath::Clamp(CurrentSuccesses + 1, 0, TargetSuccessCount);
        FlashZoneSuccess();

        if (SuccessSound)
        {
            UGameplayStatics::PlaySound2D(this, SuccessSound);
        }
    }
    else
    {
        CurrentSuccesses = FMath::Max(0, CurrentSuccesses - 1);
        FlashZoneFail();

        if (FailSound)
        {
            UGameplayStatics::PlaySound2D(this, FailSound);
        }
    }

    UpdateRemainingText();

    if (CurrentSuccesses >= TargetSuccessCount)
    {
        StopMiniGame(true);
    }
    else
    {
        HideCircleAndScheduleRespawn();
    }
}

void URepairMiniGameWidget::OnCancelButtonClicked()
{
    StopMiniGame(false);
}

void URepairMiniGameWidget::StopMiniGame(bool bCompleted)
{
    bMiniGameActive = false;
    CurrentSuccesses = 0;
    bCircleActive = false;

    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(SpeedChangeTimer);
        GetWorld()->GetTimerManager().ClearTimer(FailFlashTimer);
        GetWorld()->GetTimerManager().ClearTimer(SuccessFlashTimer);
        GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
    }

    if (CheckButton)
    {
        CheckButton->SetIsEnabled(false);
    }

    OnMiniGameFinished.Broadcast(bCompleted);
}

void URepairMiniGameWidget::UpdateRemainingText()
{
    if (RemainingSuccessText)
    {
        const int32 Remaining = FMath::Max(0, TargetSuccessCount - CurrentSuccesses);
        RemainingSuccessText->SetText(FText::FromString(FString::Printf(TEXT("남은 성공: %d"), Remaining)));
    }
}

void URepairMiniGameWidget::UpdateCircleScale(float DeltaTime)
{
    if (!HammerImage || !bCircleActive || bMissedCurrentCircle) return;

    CurrentCircleScale -= CurrentShrinkSpeed * DeltaTime;
    
    HammerImage->SetRenderScale(FVector2D(CurrentCircleScale, CurrentCircleScale));

    if (CurrentCircleScale < (CurrentTargetScale - SuccessTolerance))
    {
        bMissedCurrentCircle = true;
        
        CurrentSuccesses = FMath::Max(0, CurrentSuccesses - 1);
        FlashZoneFail();
        
        if (FailSound)
        {
            UGameplayStatics::PlaySound2D(this, FailSound);
        }
        
        UpdateRemainingText();
        HideCircleAndScheduleRespawn();
    }
}

void URepairMiniGameWidget::SpawnCircle()
{
    if (!bMiniGameActive || !HammerImage) return;

    bCircleActive = true;
    bMissedCurrentCircle = false;

    CurrentTargetScale = FMath::FRandRange(SuccessTargetScaleMin, SuccessTargetScaleMax);
    if (ZoneImage)
    {
        ZoneImage->SetRenderScale(FVector2D(CurrentTargetScale, CurrentTargetScale));
    }
    
    CurrentCircleScale = CircleStartScale;
    HammerImage->SetRenderScale(FVector2D(CurrentCircleScale, CurrentCircleScale));
    HammerImage->SetVisibility(ESlateVisibility::Visible);
}

void URepairMiniGameWidget::HideCircleAndScheduleRespawn()
{
    if (!bMiniGameActive || !HammerImage || !GetWorld()) return;

    bCircleActive = false;
    HammerImage->SetVisibility(ESlateVisibility::Hidden);

    float Difficulty = 0.0f;
    if (TargetSuccessCount > 0)
    {
        Difficulty = FMath::Clamp(static_cast<float>(CurrentSuccesses) / static_cast<float>(TargetSuccessCount), 0.0f, 1.0f);
    }
    
    const float DelayScale = FMath::Lerp(1.0f, DifficultyRespawnMultiplierMin, Difficulty);
    const float DelayMin = RespawnDelayMin * DelayScale;
    const float DelayMax = RespawnDelayMax * DelayScale;
    const float Delay = FMath::FRandRange(DelayMin, DelayMax);
    
    GetWorld()->GetTimerManager().SetTimer(
        RespawnTimer,
        this,
        &URepairMiniGameWidget::SpawnCircle,
        Delay,
        false);
}

void URepairMiniGameWidget::RandomizeShrinkSpeed()
{
    if (!GetWorld()) return;

    float Difficulty = 0.0f;
    if (TargetSuccessCount > 0)
    {
        Difficulty = FMath::Clamp(static_cast<float>(CurrentSuccesses) / static_cast<float>(TargetSuccessCount), 0.0f, 1.0f);
    }
    
    const float SpeedScale = FMath::Lerp(1.0f, DifficultySpeedMultiplierMax, Difficulty);
    const float MinSpeed = ShrinkSpeedMin * SpeedScale;
    const float MaxSpeed = ShrinkSpeedMax * SpeedScale;
    CurrentShrinkSpeed = FMath::FRandRange(MinSpeed, MaxSpeed);

    const float Interval = FMath::FRandRange(SpeedChangeIntervalMin, SpeedChangeIntervalMax);
    GetWorld()->GetTimerManager().SetTimer(
        SpeedChangeTimer,
        this,
        &URepairMiniGameWidget::RandomizeShrinkSpeed,
        Interval,
        false);
}

bool URepairMiniGameWidget::IsCircleInZone() const
{
    return FMath::IsWithinInclusive(CurrentCircleScale, CurrentTargetScale - SuccessTolerance, CurrentTargetScale + SuccessTolerance);
}

void URepairMiniGameWidget::FlashZoneFail()
{
    if (!TrackImage || !GetWorld()) return;

    TrackImage->SetColorAndOpacity(FailFlashColor);
    GetWorld()->GetTimerManager().ClearTimer(FailFlashTimer);
    GetWorld()->GetTimerManager().SetTimer(
        FailFlashTimer,
        this,
        &URepairMiniGameWidget::ResetZoneColor,
        FailFlashDuration,
        false);
}

void URepairMiniGameWidget::FlashZoneSuccess()
{
    if (!TrackImage || !GetWorld()) return;

    TrackImage->SetColorAndOpacity(SuccessFlashColor);
    GetWorld()->GetTimerManager().ClearTimer(SuccessFlashTimer);
    GetWorld()->GetTimerManager().SetTimer(
        SuccessFlashTimer,
        this,
        &URepairMiniGameWidget::ResetZoneColor,
        SuccessFlashDuration,
        false);
}

void URepairMiniGameWidget::ResetZoneColor()
{
    if (!TrackImage) return;
    TrackImage->SetColorAndOpacity(ZoneOriginalColor);
}

void URepairMiniGameWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bMiniGameActive)
    {
        UpdateCircleScale(InDeltaTime);
    }
}

FReply URepairMiniGameWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::Tab)
    {
        StopMiniGame(false);
        return FReply::Handled();
    }

    return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

FReply URepairMiniGameWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::Tab)
    {
        StopMiniGame(false);
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
