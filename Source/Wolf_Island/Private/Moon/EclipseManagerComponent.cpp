// Fill out your copyright notice in the Description page of Project Settings.


#include "Moon/EclipseManagerComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UEclipseManagerComponent::UEclipseManagerComponent()
{
	SetIsReplicatedByDefault(true);
}

void UEclipseManagerComponent::Server_SetEclipseOccurred_Implementation()
{
    bEclipseOccurred = true;
    bEclipseTodayConfirmed = false;
}

void UEclipseManagerComponent::OnRep_EclipseTodayConfirmed()
{
}

void UEclipseManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UEclipseManagerComponent, bEclipseOccurred);
	DOREPLIFETIME(UEclipseManagerComponent, bEclipseTodayConfirmed);
    DOREPLIFETIME(UEclipseManagerComponent, EclipseChance);
}

void UEclipseManagerComponent::Multicast_ConfirmEclipseToday_Implementation()
{
    bEclipseTodayConfirmed = true;
}

void UEclipseManagerComponent::Server_OnNewDay_Implementation(int32 CurrentDay)
{
    // 이미 게임 중 일식이 발생했으면 스킵
    if (bEclipseOccurred) return;

    // 첫날은 0%, 매일 5%씩 증가
    EclipseChance = FMath::Clamp(CurrentDay * 0.05f, 0.0f, 0.5f);
    float Roll = FMath::FRand();

    if (Roll < EclipseChance)
    {
        Multicast_ConfirmEclipseToday();
    }
    else
    {
        bEclipseTodayConfirmed = false;
    }
}

