// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/SpectatorCameraActor.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"

ASpectatorCameraActor::ASpectatorCameraActor()
{
	PrimaryActorTick.bCanEverTick = true;

	PrimaryActorTick.TickGroup = TG_PostUpdateWork; // 카메라가 다른 액터들의 위치 업데이트 이후에 움직이도록 설정
	
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootComp);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(RootComp);
}

void ASpectatorCameraActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASpectatorCameraActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASpectatorCameraActor, TargetActor);
}

void ASpectatorCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// TargetActor가 유효한지 확인
	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green, FString::Printf(TEXT("SpectatorCamera Tick: TargetActor=%s"), *GetNameSafe(TargetActor)));

	if (!IsValid(TargetActor))
	{
		return;
	}

	FVector DesiredLoc = TargetActor->GetActorLocation() + Offset;

	FRotator DesiredRot = TargetActor->GetActorRotation();
	if (APawn* TargetPawn = Cast<APawn>(TargetActor))
	{
		DesiredRot = TargetPawn->GetViewRotation(); // 마우스 시점 동기화
	}

	FVector NewLoc = FMath::VInterpTo(GetActorLocation(), DesiredLoc, DeltaTime, InterpSpeed);
	FRotator NewRot = FMath::RInterpTo(GetActorRotation(), DesiredRot, DeltaTime, InterpSpeed);

	SetActorLocationAndRotation(NewLoc, NewRot);

	FVector TargetLoc = TargetActor->GetActorLocation();
	UE_LOG(LogTemp, Warning, TEXT("Camera at %s, Target(%s) at %s"),
		*GetActorLocation().ToString(),
		*TargetActor->GetName(),
		*TargetLoc.ToString());
}

