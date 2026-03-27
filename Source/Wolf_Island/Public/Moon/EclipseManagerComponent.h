// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EclipseManagerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WOLF_ISLAND_API UEclipseManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEclipseManagerComponent();

    // Blueprint에서 새 날 시작할 때 호출
    UFUNCTION(BlueprintCallable, Server, Reliable)
    void Server_OnNewDay(int32 CurrentDay);

    // 일식 발생 여부 (한 게임에 한 번)
    UPROPERTY(Replicated, BlueprintReadOnly)
    bool bEclipseOccurred = false;

    // 오늘 일식 발생 예정 여부
    UPROPERTY(ReplicatedUsing = OnRep_EclipseTodayConfirmed, BlueprintReadOnly)
    bool bEclipseTodayConfirmed = false;

    UPROPERTY(Replicated, BlueprintReadOnly)
    float EclipseChance = 0.0f;

    UFUNCTION(BlueprintCallable, Server, Reliable)
    void Server_SetEclipseOccurred();

private:
    UFUNCTION()
    void OnRep_EclipseTodayConfirmed();

    // Multicast로 클라이언트에 일식 시작 알림
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_ConfirmEclipseToday();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
