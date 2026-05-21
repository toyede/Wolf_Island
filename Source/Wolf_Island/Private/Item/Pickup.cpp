#include "Item/Pickup.h"

#include "Character/MainPlayer.h"
#include "Components/InventoryComponent.h"
#include "Components/SphereComponent.h"
#include "Data/ItemDataStruct.h"
#include "Net/UnrealNetwork.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

APickup::APickup()
{
    bReplicates = true;
    
    PlayerDetector = CreateDefaultSubobject<USphereComponent>("PlayerDetector");
    
    PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>("PickupMesh");
    PickupMesh->SetSimulatePhysics(IsPhysics);
    PickupMesh->SetUseCCD(true);
    PickupMesh->SetCollisionProfileName("BlockAll");
    PickupMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    SetRootComponent(PickupMesh);
    
    /*static ConstructorHelpers::FObjectFinder<UDataTable>
        DT_ItemData(TEXT("/Game/item/DT_ItemData.DT_ItemData"));

    if (DT_ItemData.Succeeded())
    {
        ItemDataTable = DT_ItemData.Object;
    }*/
}

void APickup::BeginPlay()
{
    Super::BeginPlay();
    //게임 시작 시 아이템 정보 초기화
    InitializePickUp(ItemAmount);
    
    PlayerDetector->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnPlayerClose);
    PlayerDetector->OnComponentEndOverlap.AddDynamic(this, &APickup::OnPlayerOut);
}

void APickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    PlayerDetector->OnComponentBeginOverlap.RemoveDynamic(this, &APickup::OnPlayerClose);
    PlayerDetector->OnComponentEndOverlap.RemoveDynamic(this, &APickup::OnPlayerOut);
    
    Super::EndPlay(EndPlayReason);
}

void APickup::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    
    //UE_LOG(LogTemp, Warning, TEXT("[PICK UP] DURATION : %f"), InteractableData.InteractionDuration);
}

void APickup::InitializePickUp(const int32 InAmount)
{
    if (ItemHandle.DataTable && !ItemHandle.RowName.IsNone())
    {
        const FItemData* ItemData = ItemHandle.DataTable->FindRow<FItemData>(ItemHandle.RowName, ItemHandle.RowName.ToString());

        if (ItemData == nullptr) 
        {
            return;
        }

        ItemReference = FItemBaseData();
        
        ItemReference.Type = ItemData->Type;
        ItemReference.ItemID = ItemData->ID;
        ItemReference.ItemName = ItemData->TextData.Name;
        ItemReference.MaxDurability = ItemData->NumericData.Durability;
        ItemReference.CurrentDurability = ItemReference.MaxDurability;
        SetInteractionDuration(ItemData->NumericData.InteractionDuration);

        InAmount <= 0 ? ItemReference.SetAmount(1) : ItemReference.SetAmount(InAmount);

        if (ItemData->NumericData.MaxAmount < InAmount)
        {
            ItemReference.SetAmount(ItemData->NumericData.MaxAmount);
        }
       
        if (ItemData->AssetData.Mesh)
        {
            PickupMesh->SetStaticMesh(ItemData->AssetData.Mesh);
            PickupMesh->SetSimulatePhysics(IsPhysics);
        }
    }
}

void APickup::InitializeDrop(FItemBaseData ItemToDrop, const int32 InAmount)
{
    if (ItemDataTable)
    {
        const FItemData* ItemData = 
        ItemDataTable->FindRow<FItemData>(ItemToDrop.ItemID, ItemToDrop.ItemName.ToString());
        
        if (!ItemData)
        {
            UE_LOG(LogTemp, Warning, TEXT("Item Data is not valid"));
            return;
        }
        
        ItemReference = ItemToDrop;
        SetInteractionDuration(ItemData->NumericData.InteractionDuration);
        InAmount <= 0 ? ItemReference.SetAmount(1) : ItemReference.SetAmount(InAmount);
        PickupMesh->SetStaticMesh(ItemData->AssetData.Mesh);
        PickupMesh->SetSimulatePhysics(IsPhysics);
    } else
    {
        UE_LOG(LogTemp, Warning, TEXT("Item Data Table is not valid"));
    }
    
}

void APickup::OnPlayerClose(
    UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex, 
    bool bFromSweep, 
    const FHitResult& SweepResult)
{
    if (AMainPlayer* Player = Cast<AMainPlayer>(OtherActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("[ITEM] PLAYER DETECTED"))
        BeginFocus();
    } else
    {
        UE_LOG(LogTemp, Warning, TEXT("[ITEM] SOMETHING DETECTED"))
    }
}

void APickup::OnPlayerOut(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor, 
    UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex)
{
    if (AMainPlayer* Player = Cast<AMainPlayer>(OtherActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("[ITEM] PLAYER OUTTED"))
        EndFocus();
    } else
    {
        UE_LOG(LogTemp, Warning, TEXT("[ITEM] SOMETHING OUTTED"))
    }
}

void APickup::Interact_Implementation(AActor* Interactor)
{
    if (Interactor)
    {
        //인벤토리 컴포넌트 가져오기
        if (UInventoryComponent* PickerInventory = Interactor->GetComponentByClass<UInventoryComponent>())
        {
            //픽업 몽타주 실행
            if (AMainPlayer* Player = Cast<AMainPlayer>(Interactor))
            {
                if (Player->PickUpMontage)
                {
                    Player->Multi_PlayAnimMontage(Player->PickUpMontage);
                }
            }
            //그 인벤토리에 줍겠다고 요청
            PickerInventory->Request_PickUp(this);
        }
    }    
}

void APickup::BeginFocus_Implementation()
{
    Super::BeginFocus_Implementation();
    
    if (PickupMesh)
    {
        PickupMesh->SetRenderCustomDepth(true);
    }
}

void APickup::EndFocus_Implementation()
{
    Super::EndFocus_Implementation();
    
    if (PickupMesh)
    {
        PickupMesh->SetRenderCustomDepth(false);
    }
}

void APickup::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(APickup, ItemReference);
}

void APickup::OnRep_ItemReference()
{
    if (ItemDataTable)
    {
        const FItemData* ItemData = 
        ItemDataTable->FindRow<FItemData>(ItemReference.ItemID, ItemReference.ItemName.ToString());
        SetInteractionDuration(ItemData->NumericData.InteractionDuration);
        PickupMesh->SetStaticMesh(ItemData->AssetData.Mesh);
    }
}

void APickup::SaveData_Implementation(FActorSaveData& OutData)
{
    OutData.ActorID = GUID;
    OutData.Transform = GetActorTransform();
    OutData.ActorClass = GetClass();
    OutData.Velocity = GetVelocity();
	
    FMemoryWriter Writer(OutData.BinaryData, true);
    FObjectAndNameAsStringProxyArchive Ar(Writer, true);
    Ar.ArIsSaveGame = true;

    Serialize(Ar);
    
    UE_LOG(LogTemp, Warning, TEXT("[%s] Item Saved"), *GUID.ToString())
}

void APickup::LoadData_Implementation(const FActorSaveData& InData)
{
    GUID = InData.ActorID;
    SetActorTransform(InData.Transform);
	
    FMemoryReader Reader(InData.BinaryData, true);
    FObjectAndNameAsStringProxyArchive Ar(Reader, true);
    Ar.ArIsSaveGame = true;
	
    Serialize(Ar);
    
    InitializeDrop(ItemReference, ItemReference.Amount);
    
    if (IsPhysics)
    {
        PickupMesh->SetSimulatePhysics(true);
        FVector Force = PickupMesh->GetMass() * InData.Velocity;
        PickupMesh->AddImpulse(Force);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("[%s] Item Loaded"), *GUID.ToString())
    
    ForceNetUpdate();
}

//에디터에서만 실행
#if WITH_EDITOR
//에디터에서 월드에 배치된 인스턴스 아이템 코드 바꿀 때마다 업데이트 되게 하는 함수
void APickup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    const FName ChangedPropertyName = PropertyChangedEvent.Property ? FName(PropertyChangedEvent.Property->GetName()) : NAME_None;

    if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(FDataTableRowHandle, RowName))
    {
        if (ItemHandle.DataTable)
        {
            if (const FItemData* ItemData = ItemHandle.DataTable->FindRow<FItemData>(ItemHandle.RowName, ItemHandle.RowName.ToString()))
            {
                PickupMesh->SetStaticMesh(ItemData->AssetData.Mesh);
            }
        }
    }
}
#endif