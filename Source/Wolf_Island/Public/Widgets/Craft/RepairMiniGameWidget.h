// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/ItemDataStruct.h"
#include "RepairMiniGameWidget.generated.h"

class UImage;
class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRepairMiniGameFinished, bool, bSuccess);

UCLASS()
class WOLF_ISLAND_API URepairMiniGameWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Repair|MiniGame")
    FOnRepairMiniGameFinished OnMiniGameFinished;

    UPROPERTY(VisibleAnywhere, meta=(BindWidgetOptional))
    UImage* HammerImage;

    UPROPERTY(VisibleAnywhere, meta=(BindWidgetOptional))
    UImage* ZoneImage;

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
    void UpdateHammerPosition(float DeltaTime);
    void SpawnHammer();
    void HideHammerAndScheduleRespawn();
    void RandomizeHammerSpeed();
    bool IsHammerInZone() const;
    bool GetZoneBounds(float& OutMinX, float& OutMaxX) const;
    void FlashZoneFail();
    void FlashZoneSuccess();
    void ResetZoneColor();

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float HammerMinX = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float HammerMaxX = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float HammerSpeedMin = 140.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float HammerSpeedMax = 320.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float SpeedChangeIntervalMin = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float SpeedChangeIntervalMax = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    int32 MinSuccesses = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    int32 MaxSuccesses = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float SuccessCountPerSecond = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float RespawnDelayMin = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float RespawnDelayMax = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float DifficultySpeedMultiplierMax = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float DifficultyRespawnMultiplierMin = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    FLinearColor FailFlashColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float FailFlashDuration = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    FLinearColor SuccessFlashColor = FLinearColor(0.2f, 1.0f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Repair|MiniGame")
    float SuccessFlashDuration = 0.15f;

private:
    bool bMiniGameActive = false;
    float HammerX = 0.0f;
    float HammerY = 0.0f;
    float HammerWidth = 0.0f;
    int32 HammerDirection = -1;
    float HammerSpeed = 120.0f;
    bool bHammerActive = false;
    bool bMissedCurrentHammer = false;
    float PrevHammerCenterX = 0.0f;
    bool bWasInZone = false;
    int32 TargetSuccesses = 0;
    int32 CurrentSuccesses = 0;
    FRepairRecipeData CurrentRepairData;
    FTimerHandle SpeedChangeTimer;
    FTimerHandle FailFlashTimer;
    FTimerHandle SuccessFlashTimer;
    FTimerHandle RespawnTimer;
    FLinearColor ZoneOriginalColor = FLinearColor::White;
};
