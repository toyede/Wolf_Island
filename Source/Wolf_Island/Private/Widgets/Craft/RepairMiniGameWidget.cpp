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

    const float RawTarget = CurrentRepairData.Duration * SuccessCountPerSecond;
    TargetSuccesses = FMath::CeilToInt(RawTarget);
    TargetSuccesses = FMath::Clamp(TargetSuccesses, MinSuccesses, MaxSuccesses);

    HammerDirection = -1;
    HammerSpeed = FMath::FRandRange(HammerSpeedMin, HammerSpeedMax);
    bHammerActive = false;
    bMissedCurrentHammer = false;
    PrevHammerCenterX = HammerX;
    bWasInZone = false;

    if (ZoneImage)
    {
        ZoneOriginalColor = ZoneImage->GetColorAndOpacity();
    }

    // If a track image exists, use it to define the hammer travel range.
    if (TrackImage)
    {
        if (UCanvasPanelSlot* TrackSlot = Cast<UCanvasPanelSlot>(TrackImage->Slot))
        {
            const FVector2D TrackPos = TrackSlot->GetPosition();
            const FVector2D TrackSize = TrackSlot->GetSize();
            HammerMinX = TrackPos.X;
            HammerMaxX = TrackPos.X + TrackSize.X;

            // Align zone to the track vertically if possible.
            if (ZoneImage)
            {
                if (UCanvasPanelSlot* ZoneSlot = Cast<UCanvasPanelSlot>(ZoneImage->Slot))
                {
                    FVector2D ZonePos = ZoneSlot->GetPosition();
                    ZonePos.Y = TrackPos.Y;
                    ZoneSlot->SetPosition(ZonePos);
                }
            }
        }
    }

    if (HammerImage)
    {
        if (UCanvasPanelSlot* HammerSlot = Cast<UCanvasPanelSlot>(HammerImage->Slot))
        {
            HammerWidth = HammerSlot->GetSize().X;
            if (HammerWidth <= 0.0f)
            {
                HammerWidth = HammerImage->GetDesiredSize().X;
            }

            // Keep hammer fully within the track bounds.
            if (HammerWidth > 0.0f)
            {
                HammerMaxX = FMath::Max(HammerMinX, HammerMaxX - HammerWidth);
            }

            HammerX = HammerMaxX;
            HammerY = HammerSlot->GetPosition().Y;
            HammerSlot->SetPosition(FVector2D(HammerX, HammerY));
        }
        else
        {
            HammerX = HammerMaxX;
        }
        HammerImage->SetVisibility(ESlateVisibility::Hidden);
    }

    UpdateRemainingText();

    if (CheckButton)
    {
        CheckButton->SetIsEnabled(true);
    }

    RandomizeHammerSpeed();
    SpawnHammer();
}

void URepairMiniGameWidget::OnCheckButtonClicked()
{
    if (!bMiniGameActive) return;

    if (!bHammerActive)
    {
        return;
    }

    bMissedCurrentHammer = true;

    const bool bSuccess = IsHammerInZone();
    if (bSuccess)
    {
        CurrentSuccesses = FMath::Clamp(CurrentSuccesses + 1, 0, TargetSuccesses);
        FlashZoneSuccess();

        if (SuccessSound)
        {
            UGameplayStatics::PlaySound2D(this, SuccessSound);
        }
    }
    else
    {
        // 실패 시 진행도 감소
        CurrentSuccesses = FMath::Max(0, CurrentSuccesses - 1);
        FlashZoneFail();

        if (FailSound)
        {
            UGameplayStatics::PlaySound2D(this, FailSound);
        }
    }

    UpdateRemainingText();

    if (CurrentSuccesses >= TargetSuccesses)
    {
        StopMiniGame(true);
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
    TargetSuccesses = 0;
    bHammerActive = false;

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
        const int32 Remaining = FMath::Max(0, TargetSuccesses - CurrentSuccesses);
        RemainingSuccessText->SetText(FText::FromString(FString::Printf(TEXT("남은 성공: %d"), Remaining)));
    }
}

void URepairMiniGameWidget::UpdateHammerPosition(float DeltaTime)
{
    if (!HammerImage || !bHammerActive) return;

    const float PrevCenter = HammerX + (HammerWidth * 0.5f);
    PrevHammerCenterX = PrevCenter;
    HammerX += (HammerSpeed * DeltaTime) * static_cast<float>(HammerDirection);
    const float CurrentCenter = HammerX + (HammerWidth * 0.5f);

    float ZoneMinX = 0.0f;
    float ZoneMaxX = 0.0f;
    if (!bMissedCurrentHammer && GetZoneBounds(ZoneMinX, ZoneMaxX))
    {
        const bool bInZoneNow = (CurrentCenter >= ZoneMinX && CurrentCenter <= ZoneMaxX);
        bWasInZone = bWasInZone || bInZoneNow;

        const bool bCrossedWithoutEntering = (PrevHammerCenterX > ZoneMaxX && CurrentCenter < ZoneMinX);
        const bool bExitedZone = (bWasInZone && CurrentCenter < ZoneMinX);

        if (bCrossedWithoutEntering || bExitedZone)
        {
            bMissedCurrentHammer = true;
            CurrentSuccesses = FMath::Max(0, CurrentSuccesses - 1);
            FlashZoneFail();
            
            if (FailSound)
            {
                UGameplayStatics::PlaySound2D(this, FailSound);
            }
            
            UpdateRemainingText();
            HideHammerAndScheduleRespawn();
            return;
        }
    }

    if (HammerX <= HammerMinX)
    {
        HammerX = HammerMinX;
        HideHammerAndScheduleRespawn();
        return;
    }

    if (UCanvasPanelSlot* HammerSlot = Cast<UCanvasPanelSlot>(HammerImage->Slot))
    {
        HammerSlot->SetPosition(FVector2D(HammerX, HammerY));
    }
}

void URepairMiniGameWidget::SpawnHammer()
{
    if (!bMiniGameActive || !HammerImage) return;

    bHammerActive = true;
    bMissedCurrentHammer = false;
    HammerDirection = -1;
    HammerX = HammerMaxX;
    PrevHammerCenterX = HammerX;
    bWasInZone = false;

    if (UCanvasPanelSlot* HammerSlot = Cast<UCanvasPanelSlot>(HammerImage->Slot))
    {
        HammerSlot->SetPosition(FVector2D(HammerX, HammerY));
    }

    HammerImage->SetVisibility(ESlateVisibility::Visible);
}

void URepairMiniGameWidget::HideHammerAndScheduleRespawn()
{
    if (!bMiniGameActive || !HammerImage || !GetWorld()) return;

    bHammerActive = false;
    HammerImage->SetVisibility(ESlateVisibility::Hidden);

    float Difficulty = 0.0f;
    if (TargetSuccesses > 0)
    {
        Difficulty = FMath::Clamp(static_cast<float>(CurrentSuccesses) / static_cast<float>(TargetSuccesses), 0.0f, 1.0f);
    }
    const float DelayScale = FMath::Lerp(1.0f, DifficultyRespawnMultiplierMin, Difficulty);
    const float DelayMin = RespawnDelayMin * DelayScale;
    const float DelayMax = RespawnDelayMax * DelayScale;
    const float Delay = FMath::FRandRange(DelayMin, DelayMax);
    GetWorld()->GetTimerManager().SetTimer(
        RespawnTimer,
        this,
        &URepairMiniGameWidget::SpawnHammer,
        Delay,
        false);
}

void URepairMiniGameWidget::RandomizeHammerSpeed()
{
    if (!GetWorld()) return;

    float Difficulty = 0.0f;
    if (TargetSuccesses > 0)
    {
        Difficulty = FMath::Clamp(static_cast<float>(CurrentSuccesses) / static_cast<float>(TargetSuccesses), 0.0f, 1.0f);
    }
    const float SpeedScale = FMath::Lerp(1.0f, DifficultySpeedMultiplierMax, Difficulty);
    const float MinSpeed = HammerSpeedMin * SpeedScale;
    const float MaxSpeed = HammerSpeedMax * SpeedScale;
    HammerSpeed = FMath::FRandRange(MinSpeed, MaxSpeed);

    const float Interval = FMath::FRandRange(SpeedChangeIntervalMin, SpeedChangeIntervalMax);
    GetWorld()->GetTimerManager().SetTimer(
        SpeedChangeTimer,
        this,
        &URepairMiniGameWidget::RandomizeHammerSpeed,
        Interval,
        false);
}

bool URepairMiniGameWidget::GetZoneBounds(float& OutMinX, float& OutMaxX) const
{
    if (ZoneImage)
    {
        if (const UCanvasPanelSlot* ZoneSlot = Cast<UCanvasPanelSlot>(ZoneImage->Slot))
        {
            const FVector2D ZonePos = ZoneSlot->GetPosition();
            const FVector2D ZoneSize = ZoneSlot->GetSize();
            OutMinX = ZonePos.X;
            OutMaxX = ZonePos.X + ZoneSize.X;
            return true;
        }
    }

    const float Range = FMath::Max(1.0f, HammerMaxX - HammerMinX);
    OutMinX = HammerMinX + Range * 0.4f;
    OutMaxX = HammerMinX + Range * 0.6f;
    return true;
}

bool URepairMiniGameWidget::IsHammerInZone() const
{
    float HammerCenterX = HammerX;
    if (HammerWidth > 0.0f)
    {
        HammerCenterX = HammerX + (HammerWidth * 0.5f);
    }

    float ZoneMin = 0.0f;
    float ZoneMax = 0.0f;
    if (!GetZoneBounds(ZoneMin, ZoneMax))
    {
        return false;
    }

    return HammerCenterX >= ZoneMin && HammerCenterX <= ZoneMax;
}

void URepairMiniGameWidget::FlashZoneFail()
{
    if (!ZoneImage || !GetWorld()) return;

    ZoneImage->SetColorAndOpacity(FailFlashColor);
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
    if (!ZoneImage || !GetWorld()) return;

    ZoneImage->SetColorAndOpacity(SuccessFlashColor);
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
    if (!ZoneImage) return;
    ZoneImage->SetColorAndOpacity(ZoneOriginalColor);
}

void URepairMiniGameWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bMiniGameActive)
    {
        UpdateHammerPosition(InDeltaTime);
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
