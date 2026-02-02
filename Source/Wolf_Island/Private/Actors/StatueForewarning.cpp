// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/StatueForewarning.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/MainPlayer.h"  // 경로 맞게 수정
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"

AStatueForewarning::AStatueForewarning()
{
	PrimaryActorTick.bCanEverTick = false;

	// 즉사 판정 영역
	KillZone = CreateDefaultSubobject<USphereComponent>(TEXT("KillZone"));
	RootComponent = KillZone;
	KillZone->SetSphereRadius(150.f);
	KillZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	KillZone->SetCollisionResponseToAllChannels(ECR_Overlap);

	// 이펙트 컴포넌트
	ForewarningEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ForewarningEffect"));
	ForewarningEffectComponent->SetupAttachment(RootComponent);
	ForewarningEffectComponent->bAutoActivate = false;
}

void AStatueForewarning::BeginPlay()
{
	Super::BeginPlay();

	// 전조 이펙트 시작
	if (ForewarningEffect && ForewarningEffectComponent)
	{
		ForewarningEffectComponent->SetAsset(ForewarningEffect);
		ForewarningEffectComponent->Activate();
	}

	// 타이머 시작
	GetWorldTimerManager().SetTimer(
		ForewarningTimerHandle,
		this,
		&AStatueForewarning::OnForewarningEnd,
		ForewarningDuration,
		false
	);
}

void AStatueForewarning::OnForewarningEnd()
{
	KillOverlappingPlayers();

	OnForewarningComplete.Broadcast();

	Destroy();
}

void AStatueForewarning::KillOverlappingPlayers()
{
	TArray<AActor*> OverlappingActors;
	KillZone->GetOverlappingActors(OverlappingActors, AMainPlayer::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		if (AMainPlayer* Player = Cast<AMainPlayer>(Actor))
		{
			// 즉사 데미지 (매우 큰 값)
			Player->TakeDamage(99999.f, FDamageEvent(), nullptr, this);
		}
	}
}