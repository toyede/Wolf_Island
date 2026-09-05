
// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/BuildingComponent.h"

#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Components/InventoryComponent.h"
#include "Games/MainGameState.h"
#include "Interface/BuildingInterface.h"
#include "Character/MainPlayer.h"
#include "WaterBodyActor.h"
#include "WaterBodyComponent.h"
#include "WaterBodyTypes.h"
#include "Engine/OverlapResult.h"

// Sets default values for this component's properties
UBuildingComponent::UBuildingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	// ...
}


// Called when the game starts
void UBuildingComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
	
}

void UBuildingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	
	Super::EndPlay(EndPlayReason);
}

void UBuildingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UBuildingComponent, CurrentState);
}


void UBuildingComponent::FinishBuild()
{
	if (PreviewActor)
	{
		// 프리뷰는 이미 경사 정렬된 상태다. 서버에서 정렬을 한 번만 하도록 정렬 전 Yaw를 보낸다.
		const FTransform BuildTransform(FRotator(0.0f, PreviewYaw, 0.0f), PreviewActor->GetActorLocation());
		Server_RequestBuild(CurrentRecipe, CurrentBuildData, BuildTransform);
        
		CompleteBuild(); 
	}
}

// Called every frame
void UBuildingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn && OwnerPawn->IsLocallyControlled() && CurrentState == EBuildingState::Placing)
	{
		UpdatePreview();
	}
	else if (OwnerPawn && OwnerPawn->IsLocallyControlled() && CurrentState == EBuildingState::Building)
	{
		UpdateBuildProgress();
	}
}

void UBuildingComponent::ExecuteSpawn(FRecipeData Recipe, FBuildingData BuildData, FTransform SpawnTransform)
{
	FActorSpawnParameters Params;
	Params.Instigator = Cast<APawn>(GetOwner());
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedBuilding = GetWorld()->SpawnActor<AActor>(BuildData.BuildingClass, SpawnTransform, Params);

	if (SpawnedBuilding)
	{
	}

	CurrentState = EBuildingState::Idle;
}

void UBuildingComponent::Server_EnterBuildMode_Implementation(FRecipeData Recipe, FBuildingData BuildData)
{
	
	CurrentRecipe = Recipe;
	CurrentBuildData = BuildData;
	CurrentState = EBuildingState::Placing;
}

void UBuildingComponent::Server_CancelBuild_Implementation()
{
	GetWorld()->GetTimerManager().ClearTimer(BuildTimerHandle);
	CurrentState = EBuildingState::Idle;
}



void UBuildingComponent::UpdatePreview()
{
	if (!PreviewActor) return;

	APlayerController* PC = Cast<APlayerController>(Cast<APawn>(GetOwner())->GetController());
	if (!PC) return;

	FVector CameraLoc;
	FRotator CameraRot;
	PC->GetPlayerViewPoint(CameraLoc, CameraRot);

	FVector Start = CameraLoc;
	FVector End = Start + (CameraRot.Vector() * PlacementTraceDistance);

	FHitResult Hit;
	// 물은 통과시켜 실제 지면(물속 바닥 포함)을 찾는다.
	if (TracePlacementSurface(Start, End, Hit))
	{
		const FVector TargetLocation = FVector(
			FMath::GridSnap(Hit.Location.X, 50.f), 
			FMath::GridSnap(Hit.Location.Y, 50.f), 
			Hit.Location.Z + 5.0f
		);

		const FTransform BaseTransform(FRotator(0.0f, PreviewYaw, 0.0f), TargetLocation);

		// 물속 건축물만 스폰과 같은 함수로 지면 경사에 맞춰 보여준다.
		// 육상 건축물은 기존처럼 Yaw만 적용한다(프리뷰 모양을 바꾸지 않기 위해).
		if (CurrentBuildData.PlacementTag == TEXT("Water"))
		{
			PreviewActor->SetActorTransform(AdjustSpawnTransformToGround(BaseTransform));
		}
		else
		{
			PreviewActor->SetActorTransform(BaseTransform);
		}

		bHasValidPlacementSurface = true;

		bool bIsGreen = CheckPlacementValid();
		if (PreviewActor->Implements<UBuildingInterface>())
		{
			IBuildingInterface::Execute_UpdateGhostVisual(PreviewActor, bIsGreen);
		}
	}
	else
	{
		// 아무것도 맞지 않으면 고스트가 직전 위치에 그대로 남아 화면에서 사라진 것처럼 보인다.
		// 조준선 끝으로 옮기고 배치 불가로 표시한다.
		bHasValidPlacementSurface = false;

		FVector FallbackLocation = End;

		// 깊은 물을 겨냥하면 조준선 끝이 수면 아래라 고스트가 물에 가려 안 보인다.
		// 이럴 때는 수면 위로 끌어올려 "여기는 안 된다"는 표시가 보이게 한다.
		float FallbackDepth = 0.0f;
		if (UWaterBodyComponent* WaterBodyComponent = FindSubmergedWaterBody(End, FallbackDepth))
		{
			FVector SurfaceLocation, SurfaceNormal, SurfaceVelocity;
			float SurfaceDepth = 0.0f;
			WaterBodyComponent->GetWaterSurfaceInfoAtLocation(
				End, SurfaceLocation, SurfaceNormal, SurfaceVelocity, SurfaceDepth);
			FallbackLocation.Z = SurfaceLocation.Z + 5.0f;
		}

		PreviewActor->SetActorLocation(FallbackLocation);

		if (PreviewActor->Implements<UBuildingInterface>())
		{
			IBuildingInterface::Execute_UpdateGhostVisual(PreviewActor, false);
		}
	}
}

void UBuildingComponent::CompleteBuild()
{
	CleanupBuildMode();
}

bool UBuildingComponent::CheckPlacementValid() const
{
	if (!PreviewActor) 
	{
		return false;
	}

	// 트레이스가 지면/수면을 찾지 못한 상태에서는 배치를 허용하지 않는다.
	if (!bHasValidPlacementSurface)
	{
		return false;
	}

	TArray<AActor*> OverlappingActors;
	PreviewActor->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (!Actor || Actor == PreviewActor || Actor == GetOwner()) continue;
		if (Actor->GetOwner() == GetOwner() || Actor->IsAttachedTo(GetOwner())) continue;
       
		if (Actor->ActorHasTag(TEXT("Floor")) || Actor->GetName().Contains(TEXT("Landscape")) || Actor->ActorHasTag(TEXT("Water"))) continue;

		// 워터바디는 장애물이 아니라 배치면이므로 제외한다.
		if (Actor->IsA<AWaterBody>()) continue;

		return false;
	}

	// 통발처럼 물속 바닥에 놓이는 건축물은 트레이스로 수면을 맞출 수 없으므로
	// 배치 지점이 물에 잠겨 있는지를 Water 플러그인에 직접 물어본다.
	float WaterDepth = 0.0f;
	const bool bIsWater = FindSubmergedWaterBody(PreviewActor->GetActorLocation(), WaterDepth) != nullptr;

	if (CurrentBuildData.PlacementTag == TEXT("Water"))
	{
		if (!bIsWater)
		{
			return false;
		}

		// 너무 깊으면 고스트가 수면에 가려 보이지 않으므로 배치를 막는다.
		if (MaxPlacementWaterDepth > 0.0f && WaterDepth > MaxPlacementWaterDepth)
		{
			return false;
		}
	}
    
	if (CurrentBuildData.PlacementTag == TEXT("Land") && bIsWater)
	{
		return false; 
	}

	return true;
}


bool UBuildingComponent::TracePlacementSurface(const FVector& Start, const FVector& End, FHitResult& OutHit) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	if (PreviewActor)
	{
		Params.AddIgnoredActor(PreviewActor);
	}

	// Visibility 채널은 WaterBodyCollision 프로필이 Ignore이므로 물을 통과해 물속 바닥을 찾는다.
	// 물에 잠겼는지 여부는 FindSubmergedWaterBody()가 Water 플러그인에 따로 질의한다.
	return World->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);
}


UWaterBodyComponent* UBuildingComponent::FindSubmergedWaterBody(const FVector& Location, float& OutImmersionDepth) const
{
	OutImmersionDepth = 0.0f;

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// 주의: QueryWaterInfoClosestToWorldLocation은 호수/바다처럼 평평한 수면에 대해
	// 수평 범위를 전혀 보지 않는다. 수면 높이를 GetComponentLocation().Z로 잡고
	// ImmersionDepth = 수면Z - 조회Z 로만 계산하므로(WaterBodyComponent.cpp:649,805),
	// 월드 어디든 수면 Z보다 낮기만 하면 "물속"으로 나온다.
	// 그래서 콜리전 볼륨 안에 실제로 들어있는지를 먼저 확인한다.
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	if (PreviewActor)
	{
		Params.AddIgnoredActor(PreviewActor);
	}

	// WaterBodyCollision 프로필은 WorldDynamic에 Overlap으로 응답한다.
	// 겹침만 있고 블로킹은 없으므로 반환값이 아니라 결과 배열을 본다.
	World->OverlapMultiByChannel(Overlaps, Location, FQuat::Identity, ECC_WorldDynamic,
		FCollisionShape::MakeSphere(1.0f), Params);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		const AWaterBody* WaterBody = Cast<AWaterBody>(Overlap.GetActor());
		if (!WaterBody)
		{
			continue;
		}

		UWaterBodyComponent* WaterBodyComponent = WaterBody->GetWaterBodyComponent();
		if (!WaterBodyComponent)
		{
			continue;
		}

		// 볼륨 안에 있는 것이 확인됐으니 이제 수면 아래인지(침수 깊이 > 0)만 본다.
		const FWaterBodyQueryResult QueryResult =
			WaterBodyComponent->QueryWaterInfoClosestToWorldLocation(Location, EWaterBodyQueryFlags::ComputeImmersionDepth);

		if (QueryResult.IsInWater())
		{
			OutImmersionDepth = QueryResult.GetImmersionDepth();
			return WaterBodyComponent;
		}
	}

	return nullptr;
}


void UBuildingComponent::CleanupBuildMode()
{
	if (AMainPlayer* OwnerPlayer = Cast<AMainPlayer>(GetOwner()))
	{
		if (OwnerPlayer->IsLocallyControlled())
		{
			OwnerPlayer->SetBuildingInputBlocked(false);
		}
	}

	if (PreviewActor)
	{
		FString ActorName = PreviewActor->GetName();
		PreviewActor->Destroy();
		PreviewActor = nullptr;
        
	}
	BuildStartTime = 0.0f;
	BuildDuration = 0.0f;
	bHasValidPlacementSurface = false;
	CurrentState = EBuildingState::Idle;

	if (OnBuildingModeEnded.IsBound())
	{
		OnBuildingModeEnded.Broadcast();
	}
}

void UBuildingComponent::Server_RequestBuild_Implementation(FRecipeData Recipe, FBuildingData BuildData,
                                                            FTransform SpawnTransform)
{
	UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
    
	if (Inventory && Inventory->ConsumeRecipeIngredients(Recipe))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
        
		if (BuildData.BuildingClass)
		{
			const FTransform AdjustedTransform = AdjustSpawnTransformToGround(SpawnTransform);
			GetWorld()->SpawnActor<AActor>(BuildData.BuildingClass, AdjustedTransform, SpawnParams);
			CleanupBuildMode();
		}
	}
}

bool UBuildingComponent::Server_RequestBuild_Validate(FRecipeData Recipe, FBuildingData BuildData,
	FTransform SpawnTransform)
{
	// 카메라는 소유자보다 앞/뒤로 떨어져 있으므로 트레이스 거리에 여유분을 더해 검증한다.
	// 이 검증에 실패하면 UE가 클라이언트 연결을 끊으므로 트레이스 거리와 반드시 같이 움직여야 한다.
	float Distance = FVector::Dist(GetOwner()->GetActorLocation(), SpawnTransform.GetLocation());
    if (Distance > PlacementTraceDistance + PlacementValidationMargin) return false;
    
    return true;
}

void UBuildingComponent::EnterBuildMode(const FRecipeData& Recipe, const FBuildingData& BuildData)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled()) 
	{
		return; 
	}
	
	CurrentRecipe = Recipe;
	CurrentBuildData = BuildData;
	CurrentState = EBuildingState::Placing;
	bHasValidPlacementSurface = false;
	PreviewYaw = 0.0f;

	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_EnterBuildMode(Recipe, BuildData);
	}

	if (PreviewActor) PreviewActor->Destroy();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
	if (PreviewClass)
	{
		PreviewActor = GetWorld()->SpawnActor<AActor>(PreviewClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);
        
		if (PreviewActor)
		{
			TArray<UStaticMeshComponent*> MeshComps;
			PreviewActor->GetComponents<UStaticMeshComponent>(MeshComps);

			if (MeshComps.Num() > 0 && !BuildData.GhostMesh.IsNull())
			{
				UStaticMesh* LoadedMesh = BuildData.GhostMesh.LoadSynchronous();
				if (LoadedMesh)
				{
					for (UStaticMeshComponent* Mesh : MeshComps)
					{
						if (Mesh)
						{
							Mesh->SetStaticMesh(LoadedMesh);
							Mesh->SetCastShadow(false);
							Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
							Mesh->SetCollisionResponseToAllChannels(ECR_Overlap);
							Mesh->SetGenerateOverlapEvents(true);
						}
					}
				}
			}
		}
	}
}

void UBuildingComponent::ConfirmBuild()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled()) return;

	if (CurrentState == EBuildingState::Placing && CheckPlacementValid())
	{
		CurrentState = EBuildingState::Building;
		if (AMainPlayer* OwnerPlayer = Cast<AMainPlayer>(OwnerPawn))
		{
			OwnerPlayer->SetBuildingInputBlocked(true);
		}

		float BuildTime = CurrentRecipe.Duration;
		BuildStartTime = GetWorld()->GetTimeSeconds();
		BuildDuration = BuildTime;
		UpdateBuildProgress();

		if (BuildTime > 0.0f)
		{
			GetWorld()->GetTimerManager().SetTimer(BuildTimerHandle, this, &UBuildingComponent::FinishBuild, BuildTime, false);
		}
		else
		{
			FinishBuild();
		}
	}
}

void UBuildingComponent::CancelBuild()
{
	CleanupBuildMode();
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		Server_CancelBuild();
	}
}

void UBuildingComponent::UpdateBuildProgress()
{
	if (!PreviewActor || BuildDuration <= 0.0f)
	{
		return;
	}

	const float Elapsed = GetWorld()->GetTimeSeconds() - BuildStartTime;
	const float Progress = FMath::Clamp(Elapsed / BuildDuration, 0.0f, 1.0f);

	if (PreviewActor->Implements<UBuildingInterface>())
	{
		IBuildingInterface::Execute_UpdateBuildProgress(PreviewActor, Progress);
	}
}

FTransform UBuildingComponent::AdjustSpawnTransformToGround(const FTransform& InTransform) const
{
	if (!GetWorld())
	{
		return InTransform;
	}

	const FVector Origin = InTransform.GetLocation();
	const FVector TraceUp(0.0f, 0.0f, 500.0f);
	const FVector TraceDown(0.0f, 0.0f, 2000.0f);

	float ExtentX = 50.0f;
	float ExtentY = 50.0f;
	if (PreviewActor)
	{
		TArray<UStaticMeshComponent*> MeshComps;
		PreviewActor->GetComponents<UStaticMeshComponent>(MeshComps);
		if (MeshComps.Num() > 0 && MeshComps[0] && MeshComps[0]->GetStaticMesh())
		{
			const FVector LocalExtent = MeshComps[0]->GetStaticMesh()->GetBounds().BoxExtent;
			const FVector Scale = MeshComps[0]->GetComponentScale();
			ExtentX = LocalExtent.X * Scale.X;
			ExtentY = LocalExtent.Y * Scale.Y;
		}
	}

	const FQuat BaseRot = InTransform.GetRotation();
	const FVector Right = BaseRot.GetRightVector();
	const FVector Forward = BaseRot.GetForwardVector();

	TArray<FVector> SampleOffsets;
	SampleOffsets.Add(FVector::ZeroVector);
	SampleOffsets.Add(Forward * ExtentX + Right * ExtentY);
	SampleOffsets.Add(Forward * ExtentX - Right * ExtentY);
	SampleOffsets.Add(-Forward * ExtentX + Right * ExtentY);
	SampleOffsets.Add(-Forward * ExtentX - Right * ExtentY);

	TArray<FHitResult> Hits;
	Hits.Reserve(SampleOffsets.Num());

	for (const FVector& Offset : SampleOffsets)
	{
		const FVector Start = Origin + Offset + TraceUp;
		const FVector End = Origin + Offset - TraceDown;

		FHitResult Hit;
		// 물은 통과시켜 물속 바닥 경사에 맞춘다.
		if (TracePlacementSurface(Start, End, Hit))
		{
			Hits.Add(Hit);
		}
	}

	if (Hits.Num() == 0)
	{
		return InTransform;
	}

	// Average hit point and normal for a stable plane fit.
	FVector AvgPoint(0.0f, 0.0f, 0.0f);
	FVector AvgNormal(0.0f, 0.0f, 0.0f);
	for (const FHitResult& Hit : Hits)
	{
		AvgPoint += Hit.ImpactPoint;
		AvgNormal += Hit.ImpactNormal;
	}
	AvgPoint /= static_cast<float>(Hits.Num());
	AvgNormal = AvgNormal.GetSafeNormal();
	if (AvgNormal.IsNearlyZero())
	{
		AvgNormal = Hits[0].ImpactNormal.GetSafeNormal();
	}

	const float Yaw = InTransform.GetRotation().Rotator().Yaw;
	const FVector YawForward = FRotator(0.0f, Yaw, 0.0f).Vector();
	const FRotator AlignedRot = FRotationMatrix::MakeFromZX(AvgNormal, YawForward).Rotator();

	// Push along normal so at least one sampled point touches ground (reduces floating).
	const FQuat AlignedQuat = AlignedRot.Quaternion();
	float MinDist = 0.0f;
	for (int32 i = 0; i < Hits.Num(); ++i)
	{
		const FVector Predicted = AvgPoint + AlignedQuat.RotateVector(SampleOffsets[i]);
		const float Dist = FVector::DotProduct(Hits[i].ImpactPoint - Predicted, AvgNormal);
		if (Dist < MinDist)
		{
			MinDist = Dist;
		}
	}

	FTransform Out = InTransform;
	Out.SetRotation(AlignedQuat);
	Out.SetLocation(AvgPoint + (AvgNormal * MinDist));
	return Out;
}

void UBuildingComponent::SendDebugChat(FString Message)
{
	if (AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>())
	{
		FChattingData DebugMsg;
		DebugMsg.Name = TEXT("System_Debug");
		DebugMsg.Message = Message;
		GS->AddChattingMessage(DebugMsg);
	}
}

void UBuildingComponent::RotatePreview(float AxisValue)
{
	if (CurrentState != EBuildingState::Placing || !PreviewActor)
	{
		return;
	}

	if (FMath::IsNearlyZero(AxisValue))
	{
		return;
	}

	const float Step = PreviewRotationStepDegrees;
	const float DeltaYaw = (AxisValue > 0.0f) ? Step : -Step;
	// 실제 회전은 UpdatePreview가 경사 정렬과 함께 적용한다.
	PreviewYaw = FMath::Fmod(PreviewYaw + DeltaYaw + 360.0f, 360.0f);
}


