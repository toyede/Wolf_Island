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
			GetWorld()->SpawnActor<AActor>(BuildData.BuildingClass, SpawnTransform, SpawnParams);
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

			if (MeshComps.Num() > 0 && BuildData.GhostMesh.IsValid())
			{
				UStaticMesh* LoadedMesh = BuildData.GhostMesh.LoadSynchronous();
        
				for (UStaticMeshComponent* Mesh : MeshComps)
				{
					if (Mesh)
					{
						Mesh->SetStaticMesh(LoadedMesh);
						Mesh->SetCastShadow(false);
						Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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


