// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Pickup.h"

#include "Components/InventoryComponent.h"
#include "Item/ItemBase.h"
#include "Data/ItemDataStruct.h"

APickup::APickup()
{
    PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>("PickupMesh");
    PickupMesh->SetSimulatePhysics(IsPhysics);
    PickupMesh->SetUseCCD(true);
    PickupMesh->SetCollisionProfileName("BlockAll");
    PickupMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    SetRootComponent(PickupMesh);
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
        UE_LOG(LogTemp, Warning, TEXT("Item Init Start"));
        const FItemData* ItemData = ItemHandle.DataTable->FindRow<FItemData>(ItemHandle.RowName, ItemHandle.RowName.ToString());
	
        ItemReference = NewObject<UItemBase>(this, BaseClass);

        ItemReference->ID = ItemData->ID;
        ItemReference->Type = ItemData->Type;
        ItemReference->NumericData = ItemData->NumericData;
        ItemReference->TextData = ItemData->TextData;
        ItemReference->AssetData = ItemData->AssetData;

        InAmount <= 0 ? ItemReference->SetAmount(1) : ItemReference->SetAmount(InAmount);
		
        PickupMesh->SetStaticMesh(ItemData->AssetData.Mesh);
        PickupMesh->SetSimulatePhysics(IsPhysics);
        
    }
}

void APickup::InitializeDrop(UItemBase* ItemToDrop, const int32 InAmount)
{
    ItemReference = ItemToDrop;
    InAmount <= 0 ? ItemReference->SetAmount(1) : ItemReference->SetAmount(InAmount);
    PickupMesh->SetStaticMesh(ItemToDrop->AssetData.Mesh);
}

void APickup::Interact(AActor* Interactor)
{
    if (Interactor)
    {
        PickUp(Interactor);
    }    
}

void APickup::PickUp(const AActor* Picker)
{
    if (!IsPendingKillPending())
    {
        if (ItemReference)
        {
            //인벤토리 컴포넌트 가져오기
            if (UInventoryComponent* PickerInventory = Picker->GetComponentByClass<UInventoryComponent>())
            {
                //아이템 추가 시퀀스 실행
                const FItemAddResult AddResult = PickerInventory->HandleAddItem(ItemReference);

                //결과에 따른 행동
                switch (AddResult.OperationResult)
                {
                case EItemAddResult::NoItemAdded:
                    break;
                case EItemAddResult::PartiallyItemAdded:
                    break;
                //싹 다 먹었으면 삭제
                case EItemAddResult::AllItemAdded:
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
        } else
        {
            //디버깅 결과 메시지
            UE_LOG(LogTemp, Warning, TEXT("Item Reference is invalid"));
        }
    }
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