#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StoneProjectile.generated.h"

class UProjectileMovementComponent;

UCLASS()
class WOLF_ISLAND_API AStoneProjectile : public AActor
{
    GENERATED_BODY()

public:
    AStoneProjectile();

    void Launch(const FVector& Direction, float Speed);

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

protected:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UProjectileMovementComponent> ProjectileComp;

    UPROPERTY(EditAnywhere, Category = "Damage")
    float Damage = 20.f;

    UPROPERTY(EditAnywhere, Category = "Projectile")
    float LifeSpan = 5.f;
};