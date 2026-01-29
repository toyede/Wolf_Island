#include "Item/Tree.h"
#include "Item/Pickup.h"
#include "Item/ItemBase.h"
#include "Data/ItemDataStruct.h"
#include "Components/StatusComponent.h" 
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"

ATree::ATree()
{
    PrimaryActorTick.bCanEverTick = false;

    TreeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TreeMesh"));
    SetRootComponent(TreeMesh);
    TreeMesh->SetCollisionProfileName(TEXT("BlockAll"));

    // StatusComponent 생성
    StatusComponent = CreateDefaultSubobject<UStatusComponent>(TEXT("StatusComponent"));
}

void ATree::BeginPlay()
{
    Super::BeginPlay();

    // 델리게이트 연결: HP가 0이 되면 OnTreeDestroyed 실행
    if (StatusComponent)
    {
        StatusComponent->OnHPZero.AddDynamic(this, &ATree::OnTreeDestroyed);
    }
}

float ATree::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // 데미지를 StatusComponent에 전달하여 HP 감소
    if (StatusComponent)
    {
        StatusComponent->DecreaseHP(ActualDamage);
    }

    return ActualDamage;
}

void ATree::OnTreeDestroyed()
{
    // 1. 파티클 이펙트 재생
    if (DestroyParticle)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DestroyParticle, GetActorLocation(), GetActorRotation(), true);
    }

    // 2. 사운드 재생
    if (DestroySound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DestroySound, GetActorLocation());
    }

    // 3. 아이템 드랍
    SpawnDrops();

    // 4. 나무 액터 삭제
    Destroy();
}

void ATree::SpawnDrops()
{
    if (!PickupClass) return;

    for (const FTreeDropEntry& DropEntry : DropList)
    {
        if (DropEntry.ItemHandle.DataTable && !DropEntry.ItemHandle.RowName.IsNone())
        {
            // 확률 체크
            if (FMath::FRand() <= DropEntry.DropChance)
            {
                const FItemData* ItemData = DropEntry.ItemHandle.DataTable->FindRow<FItemData>(DropEntry.ItemHandle.RowName, DropEntry.ItemHandle.RowName.ToString());

                if (ItemData)
                {
                    UItemBase* NewItemData = NewObject<UItemBase>(this, UItemBase::StaticClass());
                    NewItemData->ID = ItemData->ID;
                    NewItemData->Type = ItemData->Type;
                    NewItemData->NumericData = ItemData->NumericData;
                    NewItemData->TextData = ItemData->TextData;
                    NewItemData->AssetData = ItemData->AssetData;
                    
                    FItemBaseData VeryNewItemData = FItemBaseData();
                    VeryNewItemData.ItemID = ItemData->ID;
                    VeryNewItemData.ItemName = ItemData->TextData.Name;

                    // 위치 랜덤 오프셋 (겹치지 않게)
                    FVector RandomOffset = FMath::VRand(); 
                    RandomOffset.Z = 0.5f; 
                    RandomOffset.Normalize();
                    FVector SpawnLocation = GetActorLocation() + (RandomOffset * FMath::RandRange(50.0f, 100.0f));
                    
                    FActorSpawnParameters SpawnParams;
                    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

                    APickup* SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

                    if (SpawnedPickup)
                    {
                        SpawnedPickup->InitializeDrop(VeryNewItemData, DropEntry.Amount);
                        
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
}