// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/LightComponent.h"
#include "GameFramework/Character.h"
#include "MoonlightInfectionSystem.generated.h"

UCLASS()
class WOLF_ISLAND_API AMoonlightInfectionSystem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMoonlightInfectionSystem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// ========== Blueprint에서 조정 가능한 변수들 ==========

	/** 감염 체크 주기 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moonlight Settings")
	float CheckInterval = 0.1f;
	/** 한 번 노출될 때마다 증가하는 감염도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moonlight Settings")
	float InfectionPerCheck = 0.1f; // 0.1초마다 0.1씩 = 1초에 1
	/** 캡슐 트레이스 반지름 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moonlight Settings")
	float CapsuleRadius = 30.0f;
	/** 캡슐 트레이스 절반 높이 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moonlight Settings")
	float CapsuleHalfHeight = 88.0f; // 기본 캐릭터 캡슐과 비슷
	/** 레이 최대 거리 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moonlight Settings")
	float TraceDistance = 100000.0f;
	/** 디버그 드로우 활성화 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDrawDebugTrace = true;
	/** 디버그 메시지 출력 활성화 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugMessages = true;
	// ========== 시스템 제어 함수 (BP_DynamicSky에서 호출) ==========
	/** 감염 체크 시작 (밤 시작 시 호출) */
	UFUNCTION(BlueprintCallable, Category = "Moonlight System")
	void ActivateInfectionCheck();

	/** 감염 체크 중지 (낮 시작 시 호출) */
	UFUNCTION(BlueprintCallable, Category = "Moonlight System")
	void DeactivateInfectionCheck();

private:
	// ========== 내부 변수 ==========
	/** BP_DynamicSky 참조 */
	UPROPERTY()
	AActor* DynamicSkyActor;
	/** MoonDirectionalLight 컴포넌트 참조 */
	UPROPERTY()
	ULightComponent* MoonLight;
	/** 체크 타이머 핸들 */
	FTimerHandle CheckTimerHandle;
	// ========== 내부 함수 ==========
	/** 모든 플레이어 감염 체크 */
	void CheckAllPlayers();
	/** 개별 플레이어가 달빛에 노출되었는지 확인 */
	bool IsPlayerExposedToMoonlight(AActor* Player);
	/** 플레이어에게 감염 적용 */
	void ApplyInfection(AActor* Player, float Amount);
	/** 플레이어의 감염 체크 위치 가져오기 */
	FVector GetMoonlightCheckLocation(AActor* Player);
};
