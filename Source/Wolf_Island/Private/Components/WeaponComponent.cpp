// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/WeaponComponent.h"

#include "GameFramework/Character.h"
#include "Item/ItemBase.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);
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

void UWeaponComponent::UseWeapon_Implementation()
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());

	if (Owner)
	{
		UE_LOG(LogTemp, Warning,TEXT("Try Playing Montage"));
		UE_LOG(LogTemp, Warning, TEXT("Weapon ID is %s"), *CurrentWeapon.ID.ToString());
		Owner->PlayAnimMontage(CurrentWeapon.Montage);
	}
	
}


void UWeaponComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UWeaponComponent, IsEquipped);
	DOREPLIFETIME(UWeaponComponent, CurrentWeapon);
}

void UWeaponComponent::Request_CheckWeapon(FItemBaseData HandedItem)
{
	if (GetOwner()->HasAuthority())
	{
		CheckWeapon(HandedItem);
	} else
	{
		Server_CheckWeapon(HandedItem);		
	}
}

void UWeaponComponent::Server_CheckWeapon_Implementation(FItemBaseData HandedItem)
{
	CheckWeapon(HandedItem);
}

void UWeaponComponent::Request_EquipeWeapon(FWeaponData WeaponData)
{
	if (GetOwner()->HasAuthority())
	{
		EquipeWeapon(WeaponData);
	} else
	{
		Server_EquipeWeapon(WeaponData);
	}
}

void UWeaponComponent::Server_EquipeWeapon_Implementation(FWeaponData WeaponData)
{
	EquipeWeapon(WeaponData);
}

void UWeaponComponent::Request_UnequipeWeapon()
{
	if (GetOwner()->HasAuthority())
	{
		UnequipeWeapon();
	} else
	{
		Server_UnequipeWeapon();
	}
}

void UWeaponComponent::Server_UnequipeWeapon_Implementation()
{
	UnequipeWeapon();
}

void UWeaponComponent::Request_UseWeapon()
{
	if (GetOwner()->HasAuthority())
	{
		UseWeapon();
	} else
	{
		Server_UseWeapon();
	}
}

void UWeaponComponent::Server_UseWeapon_Implementation()
{
	UseWeapon();
}


