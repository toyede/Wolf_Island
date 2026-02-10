// Fill out your copyright notice in the Description page of Project Settings.


#include "Interaction/Chest.h"

#include "Components/InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/Chest/ChestScreen.h"

AChest::AChest()
{
	bReplicates = true;
	
	ChestSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>("ChestSkeletalMesh");
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("InventoryComponent");

	ChestSkeletalMesh->SetCollisionProfileName("BlockAll");
	
	InventoryComponent->SetSlotsCapacity(ChestSlotsSize);
	InventoryComponent->SetWeightCapacity(ChestWeightCapacity);
	
	InteractableData.InteractionDuration = InteractionDuration;
}

void AChest::BeginPlay()
{
	Super::BeginPlay();
	
	//초기 세팅 아이템이 있고 서버면 그걸로 채우기, 클라는 알아서 리플리케이션
	if (InitItems.Num()!=0)
	{
		for (FSlotItem Item : InitItems)
		{
			FItemBaseData ItemData = FItemBaseData();
			ItemData.ItemID = Item.ItemID;
			ItemData.Amount = Item.Amount;
			UE_LOG(LogTemp, Warning, TEXT("Item ID: %s"), *ItemData.ItemID.ToString());
			InventoryComponent->Request_HandleAddItem(ItemData);
		}
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("No inventory components available"));
	}
}

void AChest::OpenChest(AActor* Interactor)
{
	//누가 사용 중이면 아무것도 안함
	if (IsOccupied) return;

	//사용 중이라고 설정
	IsOccupied = true;
	
	//오너 설정
	SetOwner(Interactor);
	
	if (OpenAnim&&ChestOpenSound)
	{
		Multi_PlayAnimAndSound(OpenAnim, ChestOpenSound);
	}
	
	Client_OpenChest(Interactor);
}

void AChest::CloseChest()
{
	//사용 중 아니라고 설정
	IsOccupied = false;
	
	if (CloseAnim&&ChestCloseSound)
	{
		Multi_PlayAnimAndSound(CloseAnim, ChestCloseSound);
	}
	
	//오너 설정 해제
	SetOwner(nullptr);
}

//상자를 누군가 열었다!
void AChest::Interact(AActor* Interactor)
{
	OpenChest(Interactor);
}

void AChest::BeginFocus()
{
	Super::BeginFocus();
	
	if (ChestSkeletalMesh)
	{
		ChestSkeletalMesh->SetRenderCustomDepth(true);
	}
}

void AChest::EndFocus()
{
	Super::EndFocus();
	
	if (ChestSkeletalMesh)
	{
		ChestSkeletalMesh->SetRenderCustomDepth(false);
	}
}

void AChest::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AChest, InventoryComponent);
	DOREPLIFETIME(AChest, IsOccupied);
}

void AChest::Server_CloseChest_Implementation()
{
	CloseChest();
}

void AChest::Multi_PlayAnimAndSound_Implementation(UAnimationAsset* Anim, USoundBase* Sound)
{
	if (ChestSkeletalMesh)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
		ChestSkeletalMesh->PlayAnimation(Anim, false);
	}
}

void AChest::Client_OpenChest_Implementation(AActor* Interactor)
{
	if (!IsValid(Interactor)) return;
	if (!ChestWidgetClass) return;
	
	//상자 위젯 생성
	UChestScreen* ChestScreen = CreateWidget<UChestScreen>(GetWorld(), ChestWidgetClass);
	ChestScreen->InitializeChest(this, Interactor);
	ChestScreen->SetIsFocusable(true);
	ChestScreen->AddToViewport();
}
