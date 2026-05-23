// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/InventoryComponent.h"
#include "PlayerHUD.generated.h"

/**
 * 
 */

class UTextBlock;
class UProgressBar;
class UItemAcquiredBlock;

UCLASS()
class WOLF_ISLAND_API UPlayerHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable)
	void AddItemMessage(FItemAddResult Result);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class AMainPlayer* PlayerRef;
		
	UPROPERTY(meta=(BindWidget))
	UProgressBar* InteractionBar;
	
	UPROPERTY(meta=(BindWidget))
	class UImage* CrossHair;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UItemAcquiredBlock> ItemAcquiredBlockClass;
	
	//아이템 획득 메시지 넣는 곳
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UVerticalBox* InfoList;

	//핫바 슬롯
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UWrapBox* HotBar;
	
	//상태 프로그래스 바들
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UProgressBar* HealthBar;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UProgressBar* StaminaBar;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UProgressBar* HungerBar;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UProgressBar* HydrationBar;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UProgressBar* AirBar;
	
	//아이콘 애님
	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* HealthIconAnimation;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* StaminaIconAnimation;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* HungerIconAnimation;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* HydrationIconAnimation;
	
	//스크릿 엣지 해야하는 상황인가? 변수
	UPROPERTY(BlueprintReadWrite)
	bool ShouldEffect = false;

	// --- 감염 경고 스크린 이펙트 ---
	/** 플레이어 각각의 하루 당 감염 누적량이 이 값 이상이면 보라 경고 (기본 13.0) */
	UPROPERTY(EditDefaultsOnly, Category = "HUD|InfectionWarning")
	float InfectionWarningThreshold = 14.0f;

	UPROPERTY(EditDefaultsOnly, Category = "HUD|InfectionWarning")
	FLinearColor ScreenEffectInfectionWarningColor = FLinearColor(0.22f, 0.0f, 0.30f, 1.0f);
	
	//스크린 엣지 이미지
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UImage* ScreenHitImage;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UImage* ScreenEffectImage;
	
	//스크린 엣지 이펙트 애님
	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* ScreenEffectAnimation;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* ScreenHitAnimation;
	
	//공격 에임 상대의 HP
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UTextBlock* TargetHPText;
	
	//살리는 중! 안내 메시지
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UTextBlock* InteractingInfoText;
	
	//에임 중인 대상 이름
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UTextBlock* InteractableInfoText;
	
	//인터랙션 키 아이콘
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UImage* InteractableIcon;
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidgetAnim), Transient)
	UWidgetAnimation* Infected;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UInventorySlot> SlotClass;
	
	UPROPERTY(EditDefaultsOnly)
	UTexture2D* DefaultCrossHair;
	UPROPERTY(EditDefaultsOnly)
	UTexture2D* InteractableCrossHair;

	UPROPERTY()
	bool ShowInteraction = true;
	
	UFUNCTION(BlueprintCallable)
	void SetPlayerRef(AMainPlayer* OwnerPlayer);

	UFUNCTION(BlueprintCallable)
	void DisplayInteraction();
	UFUNCTION(BlueprintCallable)
	void HideInteraction();
	UFUNCTION(BlueprintCallable)
	void ToggleInteraction();
	UFUNCTION(BlueprintCallable)
	void UpdateInteraction();
	UFUNCTION(BlueprintCallable)
	void DisplayInteractable();
	UFUNCTION(BlueprintCallable)
	void DisplayDefault();
	UFUNCTION(BlueprintCallable)
	void DisplayAirBar();
	UFUNCTION(BlueprintCallable)
	void HideAirBar();

	UFUNCTION(BlueprintCallable)
	void RefreshHotBar();
	
	UFUNCTION(BlueprintCallable)
	void UpdateHotBar();
	
	UFUNCTION(BlueprintCallable)
	void UpdateStatusBars();
	
	UFUNCTION(BlueprintCallable)
	void OnInfectionChanged();
	
	UFUNCTION(BlueprintCallable)
	void DisplayTargetHP(AActor* Target);
	UFUNCTION(BlueprintCallable)
	void HideTargetHP();
	
	UFUNCTION(BlueprintCallable)
	void DisplayInteractionInfoText(AActor* Target);
	UFUNCTION(BlueprintCallable)
	void HideInteractionInfoText();
	
	UFUNCTION(BlueprintCallable)
	void DisplayInteractableInfoText(AActor* Target);
	UFUNCTION(BlueprintCallable)
	void DisplayInteractableInfoTextByItem(const FItemData& ItemData);
	UFUNCTION(BlueprintCallable)
	void DisplayInteractableInfoTextByComponent(UActorComponent* Component);
	UFUNCTION(BlueprintCallable)
	void HideInteractableInfoText();
	
	UFUNCTION(BlueprintCallable)
	void PlayScreenEffect(FColor Color = FColor::Red);
	UFUNCTION(BlueprintCallable)
	void PlayerScreenHit(FColor Color = FColor::Red);
	
	UFUNCTION()
	void PlayIconAnim(UWidgetAnimation* Anim);
	UFUNCTION()
	void StopIconAnim(UWidgetAnimation* Anim);

protected:
	
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
};
