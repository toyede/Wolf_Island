// Fill out your copyright notice in the Description page of Project Settings.

#include "Interaction/PortalActor.h"

#include "Character/MainPlayer.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Games/MainGameState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetMathLibrary.h"

APortalActor::APortalActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	ActiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ActiveMesh"));
	ActiveMesh->SetupAttachment(Root);
	ActiveMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InactiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InactiveMesh"));
	InactiveMesh->SetupAttachment(Root);
	InactiveMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MultiReadyVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("MultiReadyVolume"));
	MultiReadyVolume->SetupAttachment(Root);
	MultiReadyVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MultiReadyVolume->SetCollisionObjectType(ECC_WorldDynamic);
	MultiReadyVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	MultiReadyVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	MultiReadyVolume->SetGenerateOverlapEvents(true);
}

void APortalActor::BeginPlay()
{
	Super::BeginPlay();

	MultiReadyVolume->OnComponentBeginOverlap.AddDynamic(this, &APortalActor::OnReadyVolumeBeginOverlap);
	MultiReadyVolume->OnComponentEndOverlap.AddDynamic(this, &APortalActor::OnReadyVolumeEndOverlap);

	if (AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>())
	{
		GS->OnUnlockedRecordsChanged.AddDynamic(this, &APortalActor::HandleUnlockedRecordsChanged);
	}

	if (HasAuthority())
	{
		TArray<AActor*> Overlaps;
		MultiReadyVolume->GetOverlappingActors(Overlaps, AMainPlayer::StaticClass());
		for (AActor* Actor : Overlaps)
		{
			if (AMainPlayer* Player = Cast<AMainPlayer>(Actor))
			{
				PlayersInVolume.Add(Player);
			}
		}
	}

	UpdatePortalState();
}

void APortalActor::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!CanInteract)
	{
		return;
	}

	if (!IsValid(TargetPortal))
	{
		return;
	}

	AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>();
	const bool bIsMulti = GS && GS->IsMulti;

	if (bIsMulti)
	{
		if (!AreAllPlayersInVolume())
		{
			if (GS)
			{
				FChattingData Notice;
				Notice.Name = TEXT("알림");
				Notice.Message = TEXT("모든 플레이어가 준비 영역에 있어야 포탈이 작동합니다.");
				Notice.MessageType = EMessageType::NOTICE;
				GS->AddChattingMessage(Notice);
			}
			return;
		}

		TeleportAllPlayers();
		return;
	}

	TeleportPlayer(Interactor);
}

TArray<FString> APortalActor::GetRecordIDOptions() const
{
	TArray<FString> Options;
	Options.Add(TEXT("None"));

	UDataTable* RecordTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, TEXT("/Game/item/DT_UnknownRecord.DT_UnknownRecord")));

	if (RecordTable)
	{
		TArray<FName> RowNames = RecordTable->GetRowNames();
		for (const FName& RowName : RowNames)
		{
			Options.Add(RowName.ToString());
		}
	}

	return Options;
}

void APortalActor::HandleUnlockedRecordsChanged()
{
	UpdatePortalState();
}

void APortalActor::OnReadyVolumeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	if (AMainPlayer* Player = Cast<AMainPlayer>(OtherActor))
	{
		PlayersInVolume.Add(Player);
	}
}

void APortalActor::OnReadyVolumeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	if (AMainPlayer* Player = Cast<AMainPlayer>(OtherActor))
	{
		PlayersInVolume.Remove(Player);
	}
}

bool APortalActor::IsRecordUnlocked() const
{
	if (RequiredRecordID.IsEmpty() || RequiredRecordID.Equals(TEXT("None"), ESearchCase::IgnoreCase))
	{
		return true;
	}

	if (const AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>())
	{
		return GS->UnlockedRecordIDs.Contains(RequiredRecordID);
	}

	return false;
}

void APortalActor::UpdatePortalState()
{
	const bool bUnlocked = IsRecordUnlocked();

	if (HasAuthority())
	{
		CanInteract = bUnlocked;
		OnRep_CanInteract();
	}

	if (ActiveMesh)
	{
		ActiveMesh->SetVisibility(bUnlocked, true);
		ActiveMesh->SetHiddenInGame(!bUnlocked);
	}

	if (InactiveMesh)
	{
		InactiveMesh->SetVisibility(!bUnlocked, true);
		InactiveMesh->SetHiddenInGame(bUnlocked);
	}
}

bool APortalActor::AreAllPlayersInVolume() const
{
	const AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>();
	if (!GS)
	{
		return false;
	}

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (!PS)
		{
			return false;
		}

		AController* Controller = PS->GetOwner<AController>();
		if (!Controller)
		{
			return false;
		}

		AMainPlayer* PlayerPawn = Cast<AMainPlayer>(Controller->GetPawn());
		if (!PlayerPawn)
		{
			return false;
		}

		if (!PlayersInVolume.Contains(PlayerPawn))
		{
			return false;
		}
	}

	return true;
}

void APortalActor::TeleportAllPlayers()
{
	const AMainGameState* GS = GetWorld()->GetGameState<AMainGameState>();
	if (!GS || !IsValid(TargetPortal))
	{
		return;
	}

	const FVector TargetLocation = TargetPortal->GetActorLocation();
	const FRotator TargetRotation = TargetPortal->GetActorRotation();

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (!PS)
		{
			continue;
		}

		AController* Controller = PS->GetOwner<AController>();
		if (!Controller)
		{
			continue;
		}

		if (AMainPlayer* PlayerPawn = Cast<AMainPlayer>(Controller->GetPawn()))
		{
			FVector Offset = FVector::ZeroVector;
			if (bUseRandomOffset && RandomOffsetRadius > 0.0f)
			{
				const FVector Rand2D = UKismetMathLibrary::RandomUnitVector() * RandomOffsetRadius;
				Offset.X += Rand2D.X;
				Offset.Y += Rand2D.Y;
			}
			Offset.Z += SpawnZOffset;

			PlayerPawn->TeleportTo(TargetLocation + Offset, TargetRotation);
		}
	}
}

void APortalActor::TeleportPlayer(AActor* Interactor)
{
	if (!IsValid(TargetPortal))
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(Interactor);
	if (!Pawn)
	{
		return;
	}

	const FVector TargetLocation = TargetPortal->GetActorLocation();
	const FRotator TargetRotation = TargetPortal->GetActorRotation();
	FVector Offset = FVector::ZeroVector;
	if (bUseRandomOffset && RandomOffsetRadius > 0.0f)
	{
		const FVector Rand2D = UKismetMathLibrary::RandomUnitVector() * RandomOffsetRadius;
		Offset.X += Rand2D.X;
		Offset.Y += Rand2D.Y;
	}
	Offset.Z += SpawnZOffset;

	Pawn->TeleportTo(TargetLocation + Offset, TargetRotation);
}
