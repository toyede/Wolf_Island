// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/WeaponComponent.h"

#include "Animation/AnimInstance.h"
#include "Character/MainPlayer.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Item/ItemBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	// ...
}

// Called when the game starts
void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	SetIsReplicated(true);
}


// Called every frame
void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UWeaponComponent::CheckWeapon(FItemBaseData HandedItem)
{
	if (!WeaponDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponDataTable is NULL!!"));
		return;
	}
	
	if (HandedItem.IsValid())
	{
		FWeaponData* WeaponData = WeaponDataTable->FindRow<FWeaponData>(HandedItem.ItemID, "FindWeapon");

		if (WeaponData)
		{
			EquipeWeapon(*WeaponData);
			LinkWeaponAnimLayer(WeaponData->AnimLayerBlueprint);
		} else
		{
			FWeaponData* DefaultData = WeaponDataTable->FindRow<FWeaponData>(FName("EQ000"), "DefaultWeapon");
			CurrentWeapon = *DefaultData;
			UnequipeWeapon();
			LinkWeaponAnimLayer(DefaultData->AnimLayerBlueprint);
		}
	} else
	{
		FWeaponData* DefaultData = WeaponDataTable->FindRow<FWeaponData>(FName("EQ000"), "DefaultWeapon");
		CurrentWeapon = *DefaultData;
		UnequipeWeapon();
		LinkWeaponAnimLayer(DefaultData->AnimLayerBlueprint);
	}
}

void UWeaponComponent::LinkWeaponAnimLayer(TSubclassOf<UAnimInstance> LayerClass)
{
	if (!LayerClass) return;

	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	if (!Owner) return;

	USkeletalMeshComponent* Mesh = Owner->GetMesh();
	if (!Mesh) return;

	//애님 레이어 교체(즉시)
	Mesh->LinkAnimClassLayers(LayerClass);

	//교체로 인한 포즈 튐을 기존 Inertialization 노드로 부드럽게 블렌딩
	//(RequestSlotGroupInertialization이 그래프에 인어셜라이제이션 요청을 보내 다운스트림 노드가 반응)
	if (WeaponSwitchBlendTime > 0.0f)
	{
		if (UAnimInstance* AnimInst = Mesh->GetAnimInstance())
		{
			AnimInst->RequestSlotGroupInertialization(WeaponSwitchInertialGroup, WeaponSwitchBlendTime);
		}
	}
}

void UWeaponComponent::EquipeWeapon(FWeaponData WeaponData)
{
	IsEquipped = true;
	CurrentWeapon = WeaponData;
}

void UWeaponComponent::UnequipeWeapon()
{
	IsEquipped = false;
}

bool UWeaponComponent::SetRandomIndex()
{
	if (CurrentWeapon.AttackMontages.Num()==0) return false;
	if (IsAttacking) return false;
	
	PlayIndex = FMath::RandRange(0, CurrentWeapon.AttackMontages.Num()-1);
	
	return true;
}

void UWeaponComponent::UseWeapon_Implementation(int32 Index)
{
	AMainPlayer* Owner = Cast<AMainPlayer>(GetOwner());
	if (!Owner || IsAttacking)
	{
		UE_LOG(LogTemp, Warning, TEXT("OWNER OR IsAttacking FALSE"));
		return;
	}

	UAnimInstance* AnimInst = Owner->GetMesh() ? Owner->GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInst) return;
	
	IsAttacking = true;
	AttackMontage = CurrentWeapon.AttackMontages[Index];
	
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UWeaponComponent::OnAttackMontageEnded);
	
	Owner->PlayAnimMontage(AttackMontage);
	
	if (CurrentWeapon.Sounds.Num()!=0)
	{
		USoundBase* AttackSound;
		
		if (Index > CurrentWeapon.Sounds.Num()-1)
		{
			AttackSound= CurrentWeapon.Sounds[0];
		} else
		{
			AttackSound= CurrentWeapon.Sounds[Index];
		}
		//UGameplayStatics::PlaySoundAtLocation(GetWorld(), AttackSound, Owner->GetActorLocation());
		Owner->Multi_PlaySoundAtLocation(AttackSound, Owner->GetActorLocation());
	}
	
	//몽타주 실행 완료 후 공격 중 상태 변수 변경하는 델리게이트
	AnimInst->Montage_SetEndDelegate(EndDelegate, AttackMontage);	
}

//공격 몽타주 재생 끝나면 공격 중 상태 변수 다시 false로 설정하는 함수
void UWeaponComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != AttackMontage) return;

	IsAttacking = false;
	AttackMontage = nullptr;
}


void UWeaponComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UWeaponComponent, IsEquipped);
	DOREPLIFETIME(UWeaponComponent, IsAttacking);
	DOREPLIFETIME(UWeaponComponent, CurrentWeapon);
	DOREPLIFETIME(UWeaponComponent, PlayIndex);
}

void UWeaponComponent::Request_CheckWeapon(FItemBaseData HandedItem)
{
	if (GetOwner()->HasAuthority())
	{
		CheckWeapon(HandedItem);
	} else
	{
		Server_CheckWeapon(HandedItem);		
	}
}

void UWeaponComponent::Server_CheckWeapon_Implementation(FItemBaseData HandedItem)
{
	CheckWeapon(HandedItem);
}

void UWeaponComponent::Request_EquipeWeapon(FWeaponData WeaponData)
{
	if (GetOwner()->HasAuthority())
	{
		EquipeWeapon(WeaponData);
	} else
	{
		Server_EquipeWeapon(WeaponData);
	}
}

void UWeaponComponent::Server_EquipeWeapon_Implementation(FWeaponData WeaponData)
{
	EquipeWeapon(WeaponData);
}

void UWeaponComponent::Request_UnequipeWeapon()
{
	if (GetOwner()->HasAuthority())
	{
		UnequipeWeapon();
	} else
	{
		Server_UnequipeWeapon();
	}
}

void UWeaponComponent::Server_UnequipeWeapon_Implementation()
{
	UnequipeWeapon();
}

void UWeaponComponent::Request_UseWeapon()
{
	UE_LOG(LogTemp, Warning, TEXT("[%hs] REQUEST USE WEAPON"),
		GetOwner()->HasAuthority()?"SERVER":"CLIENT");
	if (GetOwner()->HasAuthority())
	{
		if (!SetRandomIndex()) return;
		UseWeapon(PlayIndex);
	} else
	{
		Server_UseWeapon();
	}
}

void UWeaponComponent::Server_UseWeapon_Implementation()
{
	if (!SetRandomIndex()) return;
	UseWeapon(PlayIndex);
}


