#include "Item/Pickup.h"

#include "Character/MainPlayer.h"
#include "Components/InventoryComponent.h"
#include "Item/ItemBase.h"
#include "Data/ItemDataStruct.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/PlayerHUD.h"

APickup::APickup()
{
    PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>("PickupMesh");
    PickupMesh->SetSimulatePhysics(IsPhysics);
    PickupMesh->SetUseCCD(true);
    PickupMesh->SetCollisionProfileName("BlockAll");
    PickupMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    SetRootComponent(PickupMesh);
    
    static ConstructorHelpers::FObjectFinder<UDataTable>
        DT_ItemData(TEXT("/Game/item/DT_ItemData.DT_ItemData"));

    if (DT_ItemData.Succeeded())
    {
        ItemDataTable = DT_ItemData.Object;
    }
    
    bReplicates = true;
}

void APickup::BeginPlay()
{
    Super::BeginPlay();
    //게임 시작 시 아이템 정보 초기화
    InitializePickUp(UItemBase::StaticClass(), ItemAmount);
}

void APickup::InitializePickUp(const TSubclassOf<UItemBase> BaseClass, const int32 InAmount)
{
    if (ItemHandle.DataTable && !ItemHandle.RowName.IsNone())
    {
        const FItemData* ItemData = ItemHandle.DataTable->FindRow<FItemData>(ItemHandle.RowName, ItemHandle.RowName.ToString());

        if (ItemData == nullptr) 
        {
            return;
        }
    
        TSubclassOf<UItemBase> ClassToUse = BaseClass;

        if (ClassToUse == nullptr)
        {
            ClassToUse = UItemBase::StaticClass();
        }

        ItemReference = FItemBaseData();
        
        ItemReference.ItemID = ItemData->ID;
        ItemReference.ItemName = ItemData->TextData.Name;
        ItemReference.CurrentDurability = ItemData->NumericData.Durability;
        InteractableData.InteractionDuration = ItemData->NumericData.InteractionDuration;

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
        
        ItemReference = ItemToDrop;
        InteractableData.InteractionDuration = ItemData->NumericData.InteractionDuration;
        InAmount <= 0 ? ItemReference.SetAmount(1) : ItemReference.SetAmount(InAmount);
        PickupMesh->SetStaticMesh(ItemData->AssetData.Mesh);
    } else
    {
        UE_LOG(LogTemp, Warning, TEXT("Item Data Table is not valid"));
    }
    
}

void APickup::Interact(AActor* Interactor)
{
    if (Interactor)
    {
        //인벤토리 컴포넌트 가져오기
        if (UInventoryComponent* PickerInventory = Interactor->GetComponentByClass<UInventoryComponent>())
        {
            PickerInventory->Request_PickUp(this);
        }
    }    
}

void APickup::PickUp(const AActor* Picker)
{
    if (!IsPendingKillPending())
    {
        //if (ItemReference)
        {
            //인벤토리 컴포넌트 가져오기
            if (UInventoryComponent* PickerInventory = Picker->GetComponentByClass<UInventoryComponent>())
            {
                //아이템 추가 시퀀스 실행
                const FItemAddResult AddResult;
                
                //TODO: 서버 호출 함수로 변경
                PickerInventory->HandleAddItem(ItemReference);

                const AMainPlayer* Player = Cast<AMainPlayer>(Picker);
                
                if (Player)
                {
                    Player->HUD->AddItemMessage(AddResult);
                }

                //결과에 따른 행동
                switch (AddResult.OperationResult)
                {
                    //아이템 추가 안됨
                    case EItemAddedResult::NoItemAdded:
                        //디버깅 결과 메시지
                        UE_LOG(LogTemp, Warning, TEXT("Didn't Eat Item"));
                        break;
                    //아이템 부분만 먹음
                    case EItemAddedResult::PartiallyItemAdded:
                        //디버깅 결과 메시지
                        UE_LOG(LogTemp, Warning, TEXT("Remain Some"));
                        break;
                    //아이템 싹싹김치
                    case EItemAddedResult::AllItemAdded:
                        //디버깅 결과 메시지
                        UE_LOG(LogTemp, Warning, TEXT("Got All Item"));
                        UGameplayStatics::PlaySound2D(GetWorld(), Player->ItemGettingSound);
                        Destroy();
                        break;
                }
                //디버깅 결과 메시지
                UE_LOG(LogTemp, Warning, TEXT("%s"), *AddResult.ResultMessage.ToString());
  
            } else
            {
                //디버깅 결과 메시지
                UE_LOG(LogTemp, Warning, TEXT("Inventory Component is invalid"));
            }
        }
    }
}

void APickup::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(APickup, ItemReference);
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