// Fill out your copyright notice in the Description page of Project Settings.


#include "Games/PlayerSlot.h"

#include "Components/ArrowComponent.h"
#include "Components/WidgetComponent.h"
#include "Games/MainPlayerState.h"
#include "Games/GameModes/LobbyGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/Lobby/PlayerCard.h"

// Sets default values
APlayerSlot::APlayerSlot()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	
	SetRootComponent(DefaultSceneRoot);
	ArrowComponent->SetupAttachment(DefaultSceneRoot);
	WidgetComponent->SetupAttachment(DefaultSceneRoot);
}

// Called when the game starts or when spawned
void APlayerSlot::BeginPlay()
{
	Super::BeginPlay();
	SetReplicates(true);
	
}

// Called every frame
void APlayerSlot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RefreshCard();
}

void APlayerSlot::AddPlayer(APlayerController* NewPlayer)
{
	PlayerController = NewPlayer;
	PlayerState = NewPlayer->GetPlayerState<AMainPlayerState>();
	
	if (PlayerVisual) PlayerVisual->Destroy();
	
	int RoleIndex = static_cast<int>(PlayerState->GetPlayerRole());
	
	if (UPlayerCard* PlayerCard = Cast<UPlayerCard>(WidgetComponent->GetWidget()))
	{
		PlayerCard->SetPlayerController(PlayerController);
	}
	
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.Owner = PlayerController;
	
	PlayerVisual = GetWorld()->SpawnActor<AActor>(
		PlayerVisualClasses[RoleIndex],
		ArrowComponent->GetComponentTransform(),
		SpawnInfo);
	
	if (ALobbyGameMode* LGM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
	{
		LGM->CheckAllPlayerReady();
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
	
	if (ALobbyGameMode* LGM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
	{
		LGM->CheckAllPlayerReady();
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

void APlayerSlot::RefreshCard()
{
	if (UPlayerCard* PlayerCard = Cast<UPlayerCard>(WidgetComponent->GetWidget()))
	{
		if (PlayerState)
		{
			PlayerCard->SetVisibility(ESlateVisibility::Visible);
			PlayerCard->UpdateCard(PlayerState);
		} else
		{
			//UE_LOG(LogTemp, Warning, TEXT("[PLAYER SLOT] NO PLAYER STATE"));
			PlayerCard->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void APlayerSlot::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(APlayerSlot, PlayerState);
	DOREPLIFETIME(APlayerSlot, PlayerVisual);
}

