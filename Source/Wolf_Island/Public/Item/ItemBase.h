// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/ItemDataStruct.h"
#include "ItemBase.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class WOLF_ISLAND_API UItemBase : public UObject
{
	GENERATED_BODY()

public:

	//여기엔 지금 소유인벤토리, 아이템 개수, 아이템 정보들이 있다.
	UItemBase();

	UPROPERTY()
	class UInventoryComponent* OwningInventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Data", meta=(UIMin=1, UIMax=64))
	int32 Amount;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Data")
	FName ID;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Data")
	EItemType Type;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Data")
	FItemTextData TextData;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Data")
	FItemNumericData NumericData;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Data")
	FItemAssetData AssetData;

	bool IsCopy;
	bool IsPickup;

	//아이템 복사본 생성
	UFUNCTION(BlueprintCallable, Category = "Item")
	UItemBase* CreateItemCopy() const;
	//슬롯 아이템 전체 무게 반환
	UFUNCTION(BlueprintCallable, Category = "Item")
	FORCEINLINE float GetItemStackWeight() const { return Amount * NumericData.Weight; };
	//아이템 무게 반환
	UFUNCTION(BlueprintCallable, Category = "Item")
	FORCEINLINE float GetItemSingleWeight() const { return NumericData.Weight; };
	//최대 수량인지 체크
	UFUNCTION(BlueprintCallable, Category = "Item")
	FORCEINLINE bool IsFullStack() const { return Amount == NumericData.MaxAmount; };
	//수량 설정
	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetAmount(const int32 NewAmount);
	//아이템 플래그 초기화
	UFUNCTION(BlueprintCallable, Category = "Item")
	void ResetItemFlags();
	//사용 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "Item")
	void Use(AActor* User);
	void Use_Implementation();

protected:
	bool operator==(const FName& OtherID) const
	{
		return ID == OtherID;
	}
};
