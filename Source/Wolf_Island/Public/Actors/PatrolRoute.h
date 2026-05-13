// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PatrolRoute.generated.h"

class USplineComponent;

UCLASS()
class WOLF_ISLAND_API APatrolRoute : public AActor
{
	GENERATED_BODY()
	
public:	
	APatrolRoute();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Patrol")
	TObjectPtr<USplineComponent> SplinePoints;

	/** 스플라인 포인트들의 중심점을 월드 좌표로 반환 */
	UFUNCTION(BlueprintCallable, Category = "Patrol")
	FVector GetSplineCenter() const;

	/** 중심점으로부터 모든 스플라인 포인트를 확장 (Scale 1.0 = 원래 위치) */
	UFUNCTION(BlueprintCallable, Category = "Patrol")
	void ExpandSplineFromCenter(float Scale);

	/** Sky 시스템에서 아침 이벤트를 수신할 때 호출 */
	UFUNCTION(BlueprintCallable, Category = "Patrol")
	void OnMorningEvent();

	/** 확장 목표 배율 (1.0 = 원래 크기) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol|Expansion")
	float ExpandTargetScale = 1.5f;

	/** 확장 애니메이션 지속 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol|Expansion")
	float ExpandDuration = 3.0f;

	/** 확장을 반복할 최대 횟수 (0 = 무제한) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol|Expansion")
	int32 MaxExpandCount = 5;

	/** 현재까지 확장된 횟수 (읽기 전용) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Patrol|Expansion")
	int32 CurrentExpandCount = 0;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	/** BeginPlay 시 저장된 각 스플라인 포인트의 중심 기준 오프셋 */
	TArray<FVector> OriginalOffsets;

	/** 현재 확장 배율 */
	float CurrentScale = 1.0f;

	/** 목표 확장 배율 */
	float TargetScale = 1.0f;

	/** 확장 진행 시간 */
	float ExpandElapsed = 0.0f;

	/** 확장 시작 배율 */
	float StartScale = 1.0f;

	/** 현재 확장 중인지 여부 */
	bool bIsExpanding = false;

	/** BeginPlay 시 캐시된 중심점 (로컬 기준) */
	FVector CachedCenter = FVector::ZeroVector;

	void CacheOriginalOffsets();
};
