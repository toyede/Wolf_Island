// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/WeaponComponent.h"

#include "GameFramework/Character.h"
#include "Item/ItemBase.h"

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UWeaponComponent::CheckWeapon(FItemBaseData HandedItem)
{
	if (!WeaponDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponDataTable is NULL!!"));
		return;
	}

	if (HandedItem.IsValid())
	{
		FWeaponData* WeaponData = WeaponDataTable->FindRow<FWeaponData>(HandedItem.ItemID, "FindWeapon");

		if (WeaponData)
		{
			EquipeWeapon(*WeaponData);
		} else
		{
			FWeaponData* DefaultData = WeaponDataTable->FindRow<FWeaponData>(FName("EQ000"), "DefaultWeapon");
			CurrentWeapon = *DefaultData;
			UnequipeWeapon();
		}
	} else
	{
		FWeaponData* DefaultData = WeaponDataTable->FindRow<FWeaponData>(FName("EQ000"), "DefaultWeapon");
		CurrentWeapon = *DefaultData;
		UnequipeWeapon();
	}
}

void UWeaponComponent::EquipeWeapon(FWeaponData WeaponData)
{
	IsEquipped = true;
	CurrentWeapon = WeaponData;
}

void UWeaponComponent::UnequipeWeapon()
{
	IsEquipped = false;
}

void UWeaponComponent::UseWeapon()
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());

	if (Owner)
	{
		UE_LOG(LogTemp,Warning,TEXT("Try Playing Montage"));
		Owner->PlayAnimMontage(CurrentWeapon.Montage);
	}
	
}


