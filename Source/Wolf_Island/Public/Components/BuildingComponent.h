// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ItemDataStruct.h"
#include "Engine/EngineTypes.h"
#include "Engine/HitResult.h"
#include "BuildingComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBuildingModeEnded);

UENUM(BlueprintType)
enum class EBuildingState : uint8
{
	Idle,
	Placing,
	Building
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WOLF_ISLAND_API UBuildingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Building")
	FOnBuildingModeEnded OnBuildingModeEnded;
	
	// Sets default values for this component's properties
	UBuildingComponent();

	void EnterBuildMode(const FRecipeData& Recipe, const FBuildingData& BuildData);

	void ConfirmBuild();
	void CancelBuild();
	void SendDebugChat(FString Message);
	void RotatePreview(float AxisValue);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void FinishBuild();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestBuild(FRecipeData Recipe, FBuildingData BuildData, FTransform SpawnTransform);

	UFUNCTION(Server, Reliable)
	void Server_CancelBuild();

	void ExecuteSpawn(FRecipeData Recipe, FBuildingData BuildData, FTransform SpawnTransform);

	EBuildingState GetCurrentState() const { return CurrentState; }

	UFUNCTION(Server, Reliable)
	void Server_EnterBuildMode(FRecipeData Recipe, FBuildingData BuildData);

	UFUNCTION(BlueprintPure, Category = "Building")
	bool IsConstructing() const { return CurrentState == EBuildingState::Building; }

private:
	UPROPERTY(Replicated)
	EBuildingState CurrentState = EBuildingState::Idle;

	UPROPERTY(EditAnywhere, Category = "Building")
	float PreviewRotationStepDegrees = 15.0f;
    
	UPROPERTY(EditAnywhere, Category = "Building")
	TSubclassOf<AActor> PreviewClass;

	// 카메라에서 고스트를 놓을 지점까지의 최대 거리.
	// Server_RequestBuild_Validate가 이 값을 기준으로 검증하므로 따로 놀지 않는다.
	UPROPERTY(EditAnywhere, Category = "Building")
	float PlacementTraceDistance = 1500.0f;

	// 카메라가 아니라 소유자 기준으로 재는 서버 검증에 주는 여유분.
	// RPC 검증 실패는 클라이언트 연결을 끊으므로 넉넉해야 한다.
	UPROPERTY(EditAnywhere, Category = "Building")
	float PlacementValidationMargin = 500.0f;

	// 물속 건축물을 놓을 수 있는 최대 수심. 0 이하면 제한 없음.
	// 너무 깊으면 고스트가 수면 아래로 가려져 보이지 않으므로 명시적으로 막는다.
	UPROPERTY(EditAnywhere, Category = "Building")
	float MaxPlacementWaterDepth = 500.0f;

	UPROPERTY()
	AActor* PreviewActor;

	FRecipeData CurrentRecipe;
	FBuildingData CurrentBuildData;
	FTimerHandle BuildTimerHandle;
	float BuildStartTime = 0.0f;
	float BuildDuration = 0.0f;

	// 이번 프레임에 지면을 실제로 찾았는지. 못 찾았으면 배치를 허용하지 않는다.
	bool bHasValidPlacementSurface = false;

	// 프리뷰 회전(Yaw). 지면 경사 정렬은 매 프레임 다시 계산하므로 사용자가 지정한 Yaw만 따로 보관한다.
	float PreviewYaw = 0.0f;

	void UpdatePreview();
	void UpdateBuildProgress();
	void CompleteBuild();
	bool CheckPlacementValid() const;
	bool TracePlacementSurface(const FVector& Start, const FVector& End, FHitResult& OutHit) const;

	// 해당 지점이 잠겨 있는 워터바디를 돌려준다. 잠겨 있지 않으면 nullptr.
	class UWaterBodyComponent* FindSubmergedWaterBody(const FVector& Location, float& OutImmersionDepth) const;
	FTransform AdjustSpawnTransformToGround(const FTransform& InTransform) const;
	void CleanupBuildMode();
};
