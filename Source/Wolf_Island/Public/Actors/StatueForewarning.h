// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StatueForewarning.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UNiagaraSystem;

DECLARE_MULTICAST_DELEGATE(FOnForewarningComplete);

UCLASS()
class WOLF_ISLAND_API AStatueForewarning : public AActor
{
	GENERATED_BODY()

public:
	AStatueForewarning();

protected:
	virtual void BeginPlay() override;

public:
	// 전조 완료 델리게이트
	FOnForewarningComplete OnForewarningComplete;

protected:
	// 전조 지속 시간
	UPROPERTY(EditAnywhere, Category = "Forewarning")
	float ForewarningDuration = 3.0f;

	// 즉사 판정 범위
	UPROPERTY(VisibleAnywhere, Category = "Forewarning")
	USphereComponent* KillZone;

	// 전조 이펙트 (나중에 설정)
	UPROPERTY(EditAnywhere, Category = "Forewarning|Effects")
	UNiagaraSystem* ForewarningEffect;

	UPROPERTY(VisibleAnywhere, Category = "Forewarning|Effects")
	UNiagaraComponent* ForewarningEffectComponent;

private:
	void OnForewarningEnd();
	void KillOverlappingPlayers();

	FTimerHandle ForewarningTimerHandle;
};