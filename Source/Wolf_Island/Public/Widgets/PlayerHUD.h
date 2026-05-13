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
	
	//공격 에임 상대의 HP
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UTextBlock* TargetHPText;
	
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

protected:
	
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
};
