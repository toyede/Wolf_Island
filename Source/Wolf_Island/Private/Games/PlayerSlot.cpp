// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/PlayerSlot.h"

#include "Components/ArrowComponent.h"
#include "Games/MainPlayerState.h"
#include "Net/UnrealNetwork.h"

// Sets default values
APlayerSlot::APlayerSlot()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	
	SetRootComponent(DefaultSceneRoot);
	ArrowComponent->SetupAttachment(DefaultSceneRoot);
}

// Called when the game starts or when spawned
void APlayerSlot::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayerSlot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlayerSlot::AddPlayer(APlayerController* NewPlayer)
{
	PlayerController = NewPlayer;
	PlayerState = NewPlayer->GetPlayerState<AMainPlayerState>();
	
	if (PlayerVisual) PlayerVisual->Destroy();
	
	int RoleIndex = static_cast<int>(PlayerState->GetPlayerRole());
	
	//if (RoleIndex == 0) return;
	
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.Owner = PlayerController;
	
	PlayerVisual = GetWorld()->SpawnActor<AActor>(
		PlayerVisualClasses[RoleIndex],
		ArrowComponent->GetComponentTransform(),
		SpawnInfo);
	
	if (PlayerVisual)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PLAYER SLOT] Character Spawned"));
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("[PLAYER SLOT] Character Not Spawned"));
	}
}

void APlayerSlot::RemovePlayer()
{
	PlayerController = nullptr;
	PlayerState = nullptr;
	
	if (PlayerVisual)
	{
		PlayerVisual->Destroy();
	}
}

void APlayerSlot::ChangeRole(ECharacterRole NewRole)
{
	int RoleIndex = static_cast<int>(NewRole);
	
	if (PlayerVisual) PlayerVisual->Destroy();
	
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.Owner = PlayerController;
	
	PlayerVisual = GetWorld()->SpawnActor<AActor>(
		PlayerVisualClasses[RoleIndex],
		ArrowComponent->GetComponentTransform(),
		SpawnInfo);
}

void APlayerSlot::RefreshSlot()
{
	if (PlayerState)
	{
		ChangeRole(PlayerState->GetPlayerRole());
	}
}

void APlayerSlot::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(APlayerSlot, PlayerVisual);
}

