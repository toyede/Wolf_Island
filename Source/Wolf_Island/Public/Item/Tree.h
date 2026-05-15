#pragma once

#include "CoreMinimal.h"
#include "Actors/SavableActor.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Games/SaveInterface.h"
#include "Tree.generated.h"

class UStatusComponent;
class APickup;
class UParticleSystem;
class USoundBase;

// 드랍 정보 구조체
USTRUCT(BlueprintType)
struct FTreeDropEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop", meta = (GetOptions = "GetItemIDs"), SaveGame)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop", SaveGame)
	int32 Amount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop", meta = (ClampMin = "0.0", ClampMax = "1.0"), SaveGame)
	float DropChance = 1.0f;
};

UCLASS()
class WOLF_ISLAND_API ATree : public ASavableActor
{
	GENERATED_BODY()
	
public:	
	ATree();

protected:
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// 대미지 전달 및 파괴 판정
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// 파괴 시 실행될 함수
	UFUNCTION()
	void OnTreeDestroyed();

	// 모든 클라이언트에서 파괴 효과를 재생하는 멀티캐스트
	UFUNCTION(NetMulticast, Reliable)
	void Multi_PlayDestroyEffects();
	
	// 서버에서 아이템을 스폰하는 함수
	void SpawnDrops();

	//GetOptions를 위한 함수
	UFUNCTION()
	TArray<FString> GetItemIDs() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* TreeMesh;
	
	UPROPERTY(ReplicatedUsing = OnRep_SetTree)
	UStaticMesh* CurrentTreeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStatusComponent* StatusComponent;
	
	// 모든 드랍 항목이 참조할 공통 데이터 테이블
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Settings")
	UDataTable* DropDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Settings")
	TArray<FTreeDropEntry> DropList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Settings")
	TSubclassOf<APickup> PickupClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UParticleSystem* DestroyParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	USoundBase* DestroySound;
	
	UFUNCTION()
	void OnRep_SetTree() { TreeMesh->SetStaticMesh(CurrentTreeMesh); };
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	//저장 관련 코드
	virtual void SaveData_Implementation(FActorSaveData& OutData0) override;
	virtual void LoadData_Implementation(const FActorSaveData& InData) override;
	
	void SetTreeMesh(UStaticMesh* NewTreeMesh);
};
