// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "WeaponComponent.generated.h"

//무기 데이터 (무기별 몽타주 재생 데이터 저장용)
USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	FName ID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	FName WeaponName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
	TObjectPtr<UAnimMontage> Montage;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WOLF_ISLAND_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsEquipped;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWeaponData CurrentWeapon;

	UFUNCTION(BlueprintCallable)
	void EquipeWeapon(FWeaponData WeaponData);
	UFUNCTION(BlueprintCallable)
	void UnequipeWeapon();
	UFUNCTION(BlueprintCallable)
	void UseWeapon();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
