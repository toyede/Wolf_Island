// Fill out your copyright notice in the Description page of Project Settings.

#include "Interaction/PortalActor.h"

#include "Character/MainPlayer.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/UnrealType.h"
#include "Net/UnrealNetwork.h"
#include "Games/MainGameState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetMathLibrary.h"
#include "Games/MainPlayerState.h"
#include "LevelSequence.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlayer.h"

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

void APortalActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APortalActor, bBossDefeated);
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

	if (TargetPortalID.IsEmpty() && IsValid(TargetPortal))
	{
		TargetPortalID = ReadPortalIDFromActor(TargetPortal);
	}

	ResolveTargetPortal();

	UpdatePortalState();
}

void APortalActor::LoadData_Implementation(const FActorSaveData& InData)
{
	Super::LoadData_Implementation(InData);

	if (TargetPortalID.IsEmpty() && IsValid(TargetPortal))
	{
		TargetPortalID = ReadPortalIDFromActor(TargetPortal);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(this, &APortalActor::ResolveTargetPortal);
	}
	else
	{
		ResolveTargetPortal();
	}
}


void APortalActor::Interact_Implementation(AActor* Interactor)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[PORTAL] DOESN'T HAVE AUTHORITY"))
		return;
	}

	if (!CanInteract)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PORTAL] CAN'T INTERACT"))
		return;
	}

	ResolveTargetPortal();
	if (!IsValid(TargetPortal))
	{
		UE_LOG(LogTemp, Warning, TEXT("[PORTAL] TARGET PORTAL IS NOT VALID"))
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
		} else
		{
			UE_LOG(LogTemp, Warning, TEXT("[PORTAL] ALL PLAYER READY"))
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

void APortalActor::OnRep_BossDefeated()
{
	if (TargetPortalID.IsEmpty() && IsValid(TargetPortal))
	{
		TargetPortalID = ReadPortalIDFromActor(TargetPortal);
	}

	ResolveTargetPortal();

	UpdatePortalState();
}

void APortalActor::OnBossDefeated()
{
	bBossDefeated = true;
	OnRep_BossDefeated();
}

void APortalActor::HandleUnlockedRecordsChanged()
{
	if (TargetPortalID.IsEmpty() && IsValid(TargetPortal))
	{
		TargetPortalID = ReadPortalIDFromActor(TargetPortal);
	}

	ResolveTargetPortal();

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
	bool bUnlocked = IsRecordUnlocked();

	if (bUnlocked && bRequiresBossDefeat)
	{
		bUnlocked = bBossDefeated;
	}

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
	ResolveTargetPortal();
	if (!GS || !IsValid(TargetPortal))
	{
		return;
	}

	const FVector TargetLocation = TargetPortal->GetActorLocation();
	const FRotator TargetRotation = TargetPortal->GetActorRotation();
	LastTriggeredPartyMembers.Reset();

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (AMainPlayerState* MPS = Cast<AMainPlayerState>(PS))
		{
			MPS->SetIsBossStage(true);
		}
		
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
			LastTriggeredPartyMembers.Add(PlayerPawn);

			FVector Offset = FVector::ZeroVector;
			if (bUseRandomOffset && RandomOffsetRadius > 0.0f)
			{
				const FVector Rand2D = UKismetMathLibrary::RandomUnitVector() * RandomOffsetRadius;
				Offset.X += Rand2D.X;
				Offset.Y += Rand2D.Y;
			}
			Offset.Z += SpawnZOffset;

			PlayerPawn->TeleportTo(TargetLocation + Offset, TargetRotation);

			if (AController* PC = PlayerPawn->GetController())
			{
				PC->SetControlRotation(TargetRotation);
			}
		}
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Portal Triggered"));
	
	if (TeleportSequence)
	{
		Multicast_PlayTeleportSequence();
	}
	else if (HasAuthority())
	{
		OnPortalTriggered.Broadcast(this);
	}
}

void APortalActor::TeleportPlayer(AActor* Interactor)
{

	ResolveTargetPortal();
	if (!IsValid(TargetPortal))
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(Interactor);
	if (!Pawn)
	{
		return;
	}
	
	if (AMainPlayerState* MPS = Cast<AMainPlayerState>(Pawn->GetPlayerState()))
	{
		MPS->SetIsBossStage(true);
	}

	LastTriggeredPartyMembers.Reset();
	if (AMainPlayer* PlayerPawn = Cast<AMainPlayer>(Pawn))
	{
		LastTriggeredPartyMembers.Add(PlayerPawn);
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

	if (TeleportSequence)
	{
		Multicast_PlayTeleportSequence();
	}
	else if (HasAuthority())
	{
		OnPortalTriggered.Broadcast(this);
	}
}

TArray<AMainPlayer*> APortalActor::GetLastTriggeredPartyMembers() const
{
	TArray<AMainPlayer*> Result;
	Result.Reserve(LastTriggeredPartyMembers.Num());

	for (AMainPlayer* Player : LastTriggeredPartyMembers)
	{
		if (IsValid(Player))
		{
			Result.Add(Player);
		}
	}

	return Result;
}

FString APortalActor::ReadPortalIDFromActor(const AActor* Actor) const
{
	if (!Actor)
	{
		return FString();
	}

	const FProperty* Prop = Actor->GetClass()->FindPropertyByName(TEXT("PortalID"));
	if (!Prop)
	{
		Prop = Actor->GetClass()->FindPropertyByName(TEXT("ID"));
	}

	if (const FStrProperty* StrProp = CastField<FStrProperty>(Prop))
	{
		return StrProp->GetPropertyValue_InContainer(Actor);
	}

	if (const FNameProperty* NameProp = CastField<FNameProperty>(Prop))
	{
		return NameProp->GetPropertyValue_InContainer(Actor).ToString();
	}

	return FString();
}

FString APortalActor::GetPortalID() const
{
	return ReadPortalIDFromActor(this);
}

void APortalActor::ResolveTargetPortal()
{
	if (TargetPortalID.IsEmpty())
	{
		return;
	}

	if (IsValid(TargetPortal))
	{
		const FString CurrentID = ReadPortalIDFromActor(TargetPortal);
		if (!CurrentID.IsEmpty() && CurrentID.Equals(TargetPortalID, ESearchCase::IgnoreCase))
		{
			return;
		}
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<APortalActor> It(World); It; ++It)
	{
		if (*It == this)
		{
			continue;
		}

		const FString OtherID = It->GetPortalID();
		if (!OtherID.IsEmpty() && OtherID.Equals(TargetPortalID, ESearchCase::IgnoreCase))
		{
			TargetPortal = *It;
			return;
		}
	}
}

void APortalActor::OnTeleportSequenceFinished()
{
	if (HasAuthority())
	{
		OnPortalTriggered.Broadcast(this);
	}
}

void APortalActor::Multicast_PlayTeleportSequence_Implementation()
{
	if (!TeleportSequence) return;

	FMovieSceneSequencePlaybackSettings PlaybackSettings;
	ALevelSequenceActor* OutActor;

	SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(GetWorld(), TeleportSequence, PlaybackSettings, OutActor);

	if (SequencePlayer)
	{
		if (HasAuthority())
		{
			SequencePlayer->OnFinished.AddDynamic(this, &APortalActor::OnTeleportSequenceFinished);
		}
		
		SequencePlayer->Play();
	}
}
