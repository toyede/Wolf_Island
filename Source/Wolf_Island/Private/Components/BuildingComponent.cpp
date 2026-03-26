// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/BuildingComponent.h"

#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Components/InventoryComponent.h"
#include "Games/MainGameState.h"
#include "Interface/BuildingInterface.h"

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
	SetIsReplicatedByDefault(true);
	// ...
	
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
		Server_RequestBuild(CurrentRecipe, CurrentBuildData, PreviewActor->GetActorTransform());
        
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
	FVector End = Start + (CameraRot.Vector() * 1000.0f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	Params.AddIgnoredActor(PreviewActor);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		FVector TargetLocation = FVector(
			FMath::GridSnap(Hit.Location.X, 50.f), 
			FMath::GridSnap(Hit.Location.Y, 50.f), 
			Hit.Location.Z + 5.0f
		);
		PreviewActor->SetActorLocation(TargetLocation);

		bool bIsGreen = CheckPlacementValid();
		if (PreviewActor->Implements<UBuildingInterface>())
		{
			IBuildingInterface::Execute_UpdateGhostVisual(PreviewActor, bIsGreen);
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

	TArray<AActor*> OverlappingActors;
	PreviewActor->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (!Actor || Actor == PreviewActor || Actor == GetOwner()) continue;
		if (Actor->GetOwner() == GetOwner() || Actor->IsAttachedTo(GetOwner())) continue;
       
		if (Actor->ActorHasTag(TEXT("Floor")) || Actor->GetName().Contains(TEXT("Landscape")) || Actor->ActorHasTag(TEXT("Water"))) continue;

		return false;
	}

	FVector Start = PreviewActor->GetActorLocation() + FVector(0, 0, 50.0f);
	FVector End = Start - FVector(0, 0, 500.0f);
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PreviewActor);
	Params.AddIgnoredActor(GetOwner());

	bool bIsWater = false;
    
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		AActor* HitActor = Hit.GetActor();
		UPrimitiveComponent* HitComp = Hit.GetComponent();

		if (HitActor)
		{
			if (HitActor->ActorHasTag(TEXT("Water")) || 
				HitActor->GetName().Contains(TEXT("Water")) ||
				(HitComp && HitComp->ComponentHasTag(TEXT("Water"))))
			{
				bIsWater = true;
			}
		}
	}

	if (CurrentBuildData.PlacementTag == TEXT("Water") && !bIsWater)
	{
		return false; 
	}
    
	if (CurrentBuildData.PlacementTag == TEXT("Land") && bIsWater)
	{
		return false; 
	}

	return true;
}


void UBuildingComponent::CleanupBuildMode()
{
	if (PreviewActor)
	{
		FString ActorName = PreviewActor->GetName();
		PreviewActor->Destroy();
		PreviewActor = nullptr;
        
	}
	BuildStartTime = 0.0f;
	BuildDuration = 0.0f;
	CurrentState = EBuildingState::Idle;
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
	float Distance = FVector::Dist(GetOwner()->GetActorLocation(), SpawnTransform.GetLocation());
    if (Distance > 1000.0f) return false;
    
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

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

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
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
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
	FRotator NewRot = PreviewActor->GetActorRotation();
	NewRot.Yaw = FMath::Fmod(NewRot.Yaw + DeltaYaw + 360.0f, 360.0f);
	PreviewActor->SetActorRotation(NewRot);
}


