// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "WeaponComponent.generated.h"

class UItemBase;
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

	UPROPERTY(EditDefaultsOnly)
	UDataTable* WeaponDataTable;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	bool IsEquipped;
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	FWeaponData CurrentWeapon;

	UFUNCTION(BlueprintCallable)
	void CheckWeapon(FItemBaseData HandedItem);
	UFUNCTION(BlueprintCallable)
	void EquipeWeapon(FWeaponData WeaponData);
	UFUNCTION(BlueprintCallable)
	void UnequipeWeapon();
	UFUNCTION(NetMulticast ,BlueprintCallable, Unreliable)
	void UseWeapon();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//멀티플레이 코드
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	//무기 확인
	UFUNCTION()
	void Request_CheckWeapon(FItemBaseData HandedItem);
	UFUNCTION(Server, Reliable)
	void Server_CheckWeapon(FItemBaseData HandedItem);
	
	//무기 장착
	UFUNCTION()
	void Request_EquipeWeapon(FWeaponData WeaponData);
	UFUNCTION(Server, Reliable)
	void Server_EquipeWeapon(FWeaponData WeaponData);
	
	//무기 해제
	UFUNCTION()
	void Request_UnequipeWeapon();
	UFUNCTION(Server, Reliable)
	void Server_UnequipeWeapon();
	
	//무기 사용
	UFUNCTION()
	void Request_UseWeapon();
	UFUNCTION(Server, Reliable)
	void Server_UseWeapon();	
};
