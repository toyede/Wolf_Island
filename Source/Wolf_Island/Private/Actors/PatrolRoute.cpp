// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/PatrolRoute.h"
#include "Components/SplineComponent.h"

APatrolRoute::APatrolRoute()
{
	PrimaryActorTick.bCanEverTick = true;

	SplinePoints = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	RootComponent = SplinePoints;
}

void APatrolRoute::BeginPlay()
{
	Super::BeginPlay();
	CacheOriginalOffsets();
}

void APatrolRoute::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsExpanding) return;

	ExpandElapsed += DeltaTime;
	float Alpha = FMath::Clamp(ExpandElapsed / ExpandDuration, 0.f, 1.f);

	// EaseInOut 보간으로 부드러운 확장
	float EasedAlpha = FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.f);
	CurrentScale = FMath::Lerp(StartScale, TargetScale, EasedAlpha);

	ExpandSplineFromCenter(CurrentScale);

	if (Alpha >= 1.0f)
	{
		bIsExpanding = false;
	}
}

FVector APatrolRoute::GetSplineCenter() const
{
	if (!SplinePoints) return GetActorLocation();

	const int32 NumPoints = SplinePoints->GetNumberOfSplinePoints();
	if (NumPoints == 0) return GetActorLocation();

	FVector Sum = FVector::ZeroVector;
	for (int32 i = 0; i < NumPoints; ++i)
	{
		Sum += SplinePoints->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
	}

	return Sum / static_cast<float>(NumPoints);
}

void APatrolRoute::ExpandSplineFromCenter(float Scale)
{
	if (!SplinePoints || OriginalOffsets.Num() == 0) return;

	const int32 NumPoints = SplinePoints->GetNumberOfSplinePoints();
	for (int32 i = 0; i < NumPoints; ++i)
	{
		// 중심으로부터 오프셋에 Scale을 곱해 새 로컬 위치 계산
		const FVector NewLocalPos = CachedCenter + OriginalOffsets[i] * Scale;
		SplinePoints->SetLocationAtSplinePoint(i, NewLocalPos, ESplineCoordinateSpace::Local, false);
	}

	// 한 번만 업데이트
	SplinePoints->UpdateSpline();
}

void APatrolRoute::OnMorningEvent()
{
	// 인스턴스 단위 비활성화 플래그 체크
	if (bDisableExpansion)
	{
		UE_LOG(LogTemp, Log, TEXT("[PatrolRoute] 확장 비활성화됨 - 이벤트 무시"));
		return;
	}

	// 최대 횟수 체크 (MaxExpandCount가 0이면 무제한)
	if (MaxExpandCount > 0 && CurrentExpandCount >= MaxExpandCount)
	{
		UE_LOG(LogTemp, Log, TEXT("[PatrolRoute] 최대 확장 횟수(%d) 도달 - 더 이상 확장하지 않음"), MaxExpandCount);
		return;
	}

	if (bIsExpanding) return;

	// 이전 크기를 유지한 채로 ExpandTargetScale만큼 추가 확장
	StartScale = CurrentScale;
	TargetScale = CurrentScale * ExpandTargetScale;
	ExpandElapsed = 0.0f;
	bIsExpanding = true;
	CurrentExpandCount++;

	UE_LOG(LogTemp, Log, TEXT("[PatrolRoute] 아침 이벤트 수신 - 확장 시작 (%d / %d) %.2f → %.2f"),
		CurrentExpandCount, MaxExpandCount, StartScale, TargetScale);
}

void APatrolRoute::CacheOriginalOffsets()
{
	if (!SplinePoints) return;

	OriginalOffsets.Empty();
	const int32 NumPoints = SplinePoints->GetNumberOfSplinePoints();

	// 로컬 공간 기준 중심점 계산
	FVector LocalSum = FVector::ZeroVector;
	for (int32 i = 0; i < NumPoints; ++i)
	{
		LocalSum += SplinePoints->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
	}
	CachedCenter = (NumPoints > 0) ? LocalSum / static_cast<float>(NumPoints) : FVector::ZeroVector;

	// 각 포인트의 중심 기준 오프셋 저장
	for (int32 i = 0; i < NumPoints; ++i)
	{
		const FVector LocalPos = SplinePoints->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
		OriginalOffsets.Add(LocalPos - CachedCenter);
	}

	CurrentScale = 1.0f;
}
