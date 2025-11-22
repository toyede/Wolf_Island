#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Tree.generated.h"

class APickup;
class UStatusComponent;
class UParticleSystem;
class USoundBase;

// 드랍 정보 구조체
USTRUCT(BlueprintType)
struct FTreeDropEntry
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDataTableRowHandle ItemHandle;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Amount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DropChance = 1.0f;
};

UCLASS()
class WOLF_ISLAND_API ATree : public AActor
{
	GENERATED_BODY()
    
public:    
	ATree();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* TreeMesh;

	// 체력 관리를 위한 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStatusComponent* StatusComponent;

	// ---------------------------------------------------
	// [이펙트 설정]
	// ---------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	UParticleSystem* DestroyParticle; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	USoundBase* DestroySound;         

	// ---------------------------------------------------
	// [아이템 드랍 설정]
	// ---------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Settings")
	TArray<FTreeDropEntry> DropList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop Settings")
	TSubclassOf<APickup> PickupClass;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

private:
	// HP 0일 때 실행될 델리게이트 함수
	UFUNCTION()
	void OnTreeDestroyed();

	void SpawnDrops();
};
