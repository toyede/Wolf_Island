#include "Item/Tree.h"
#include "Item/Pickup.h"
#include "Item/ItemBase.h"
#include "Data/ItemDataStruct.h"
#include "Components/StatusComponent.h" 
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystem.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Sound/SoundBase.h"

ATree::ATree()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	TreeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TreeMesh"));
	SetRootComponent(TreeMesh);
	TreeMesh->SetCollisionProfileName(TEXT("BlockAll"));

	StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));
	StatusComponent->ShowCurrentHP = true;
}

void ATree::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && StatusComponent)
	{
		StatusComponent->OnHPZero.AddDynamic(this, &ATree::OnTreeDestroyed);
	}
}

void ATree::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	
	if (HasAuthority() && StatusComponent)
	{
		StatusComponent->OnHPZero.RemoveDynamic(this, &ATree::OnTreeDestroyed);
	}
	
	Super::EndPlay(EndPlayReason);
}

float ATree::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority()) return 0.f;

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (StatusComponent)
	{
		StatusComponent->DecreaseHP(ActualDamage);
	}

	return ActualDamage;
}

void ATree::OnTreeDestroyed()
{
	if (!HasAuthority()) return;

	Multi_PlayDestroyEffects();
	SpawnDrops();
	Destroy();
}

void ATree::Multi_PlayDestroyEffects_Implementation()
{
	if (DestroyParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DestroyParticle, GetActorLocation(), GetActorRotation(), true);
	}

	if (DestroySound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DestroySound, GetActorLocation());
	}
}

void ATree::SpawnDrops()
{
	if (!HasAuthority() || !DropDataTable || !PickupClass) return;

	for (const FTreeDropEntry& DropEntry : DropList)
	{
		if (DropEntry.ItemID.IsNone()) continue;

		if (FMath::FRand() <= DropEntry.DropChance)
		{
			const FItemData* ItemData = DropDataTable->FindRow<FItemData>(DropEntry.ItemID, DropEntry.ItemID.ToString());

			if (ItemData)
			{
				FItemBaseData PickupData = FItemBaseData();
				PickupData.ItemID = ItemData->ID;
				PickupData.ItemName = ItemData->TextData.Name;
				PickupData.Amount = DropEntry.Amount;

				FVector RandomOffset = FMath::VRand(); 
				RandomOffset.Z = 0.5f; 
				RandomOffset.Normalize();
				FVector SpawnLocation = GetActorLocation() + (RandomOffset * FMath::RandRange(50.0f, 100.0f));
				
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				APickup* SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
				SpawnedPickup->ResetGUID();
				
				if (SpawnedPickup)
				{
					SpawnedPickup->InitializeDrop(PickupData, DropEntry.Amount);
					
					// 물리 효과 적용
					if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(SpawnedPickup->GetRootComponent()))
					{
						RootPrim->SetSimulatePhysics(true);
						RootPrim->AddImpulse(RandomOffset * 300.0f, NAME_None, true); 
					}
				}
			}
		}
	}
}

TArray<FString> ATree::GetItemIDs() const
{
	TArray<FString> Options;
	Options.Add(TEXT("None"));

	if (DropDataTable)
	{
		for (auto& It : DropDataTable->GetRowMap())
		{
			Options.Add(It.Key.ToString());
		}
	}

	return Options;
}

void ATree::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ATree, CurrentTreeMesh);
}

void ATree::SaveData_Implementation(FActorSaveData& OutData)
{
	Super::SaveData_Implementation(OutData);
	
}

void ATree::LoadData_Implementation(const FActorSaveData& InData)
{
	Super::LoadData_Implementation(InData);
	
	ForceNetUpdate();
}

void ATree::SetTreeMesh(UStaticMesh* NewTreeMesh)
{
	TreeMesh->SetStaticMesh(NewTreeMesh);
	CurrentTreeMesh = NewTreeMesh;
	
	OnRep_SetTree();
}
